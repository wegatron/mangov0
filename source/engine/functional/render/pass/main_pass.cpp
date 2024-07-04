#include "main_pass.h"

#include <engine/functional/global/engine_context.h>
#include <engine/utils/vk/commands.h>
#include <engine/utils/vk/framebuffer.h>
#include <engine/utils/vk/image.h>
#include <engine/utils/vk/pipeline.h>
#include <engine/utils/vk/resource_cache.h>
#include <engine/utils/vk/shader_module.h>

namespace mango {
void MainPass::init() {
  // create pipeline state
  auto pipeline_state = std::make_unique<GPipelineState>();
  VertexInputState vertex_input_state{
      .bindings =
          {// bindings, 3 float pos + 3 float normal + 2 float uv
           {0, 8 * sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX}},
      .attributes = {
          // attribute
          {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0}, // 3floats pos
          {1, 0, VK_FORMAT_R32G32B32_SFLOAT,
           3 * sizeof(float)}, // 3floats normal
          {2, 0, VK_FORMAT_R32G32_SFLOAT, 6 * sizeof(float)}}}; // 2 floats uv
  pipeline_state->setVertexInputState(vertex_input_state);
  pipeline_state->setInputAssemblyState({
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .primitive_restart_enable = VK_FALSE,
  });
  auto vs = std::make_shared<ShaderModule>();
  auto fs = std::make_shared<ShaderModule>();
  vs->load("shaders/static_mesh.vert");
  fs->load("shaders/forward_lighting.frag");
  pipeline_state->setShaderModules({vs, fs});

  pipeline_state->setViewportState(ViewPortState{
      .viewport_count = 1,
      .scissor_count = 1,
  });

  pipeline_state->setRasterizationState(
      {.depth_clamp_enable = false,
       .rasterizer_discard_enable = false,
       .polygon_mode = VK_POLYGON_MODE_FILL,
       //.cull_mode = VK_CULL_MODE_BACK_BIT,
       .cull_mode = VK_CULL_MODE_NONE,
       .front_face = VK_FRONT_FACE_COUNTER_CLOCKWISE,
       .depth_bias_enable = VK_FALSE});

  pipeline_state->setMultisampleState(
      {VK_SAMPLE_COUNT_1_BIT, false, 0.0f, 0xFFFFFFFF, false, false});
  // default depth stencil state, depth test enable, depth write enable, depth

  pipeline_state->setColorBlendState(
      {.attachments = {{
           .blendEnable = VK_FALSE,
           .colorWriteMask =
               VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
               VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
       }}});
  pipeline_state->setSubpassIndex(0);
  auto driver = g_engine.getDriver();
  auto resource_cache = g_engine.getResourceCache();
  render_pass_ = resource_cache->requestRenderPass(
      driver,
      {Attachment{.format = VK_FORMAT_R8G8B8A8_SRGB},
       Attachment{.format = VK_FORMAT_D24_UNORM_S8_UINT}},
      {LoadStoreInfo{}, LoadStoreInfo{}},
      {SubpassInfo{.output_attachments = {0}, .depth_stencil_attachment = 1}});
  pipeline_ = std::make_shared<GraphicsPipeline>(
      driver, resource_cache, render_pass_, std::move(pipeline_state));

  // using default color blend state: not blend
  // create descriptor set
  const auto &mat_set = pipeline_->getPipelineLayout()->getDescriptorSetLayout(1);
  
}

void MainPass::render(const std::shared_ptr<CommandBuffer> &cmd_buffer) {
  // assert(p_render_data_ != nullptr);
  auto color_img_view = frame_buffer_->getRenderTarget()->getImageViews()[0];
  color_img_view->transitionLayout(cmd_buffer->getHandle(),
                                   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
  cmd_buffer->beginRenderPass(render_pass_, frame_buffer_);
  cmd_buffer->setViewPort({VkViewport{0, static_cast<float>(height_), static_cast<float>(width_),
                                      -static_cast<float>(height_), 0.f, 1.f}});
  // cmd_buffer->setViewPort({VkViewport{0, 0, static_cast<float>(width_),
  //                                     static_cast<float>(height_), 0.f, 1.f}});  
  cmd_buffer->setScissor({VkRect2D{{0, 0}, {width_, height_}}});
  if (render_data_ != nullptr)
    draw(cmd_buffer, render_data_->static_mesh_render_data);
  cmd_buffer->endRenderPass();
  // add image barrier
  color_img_view->transitionLayout(cmd_buffer->getHandle(),
                                   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  //   ImageMemoryBarrier image_memory_barrier{
  //       .old_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
  //       .new_layout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
  //       .src_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
  //       .dst_access_mask = VK_ACCESS_SHADER_READ_BIT,
  //       .src_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
  //       .dst_stage_mask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
  //       .src_queue_family_index = VK_QUEUE_FAMILY_IGNORED,
  //       .dst_queue_family_index = VK_QUEUE_FAMILY_IGNORED};
  //   cmd_buffer->imageMemoryBarrier(
  //       image_memory_barrier,
  //       frame_buffer_->getRenderTarget()->getImageViews()[0]);
}

void MainPass::draw(
    const std::shared_ptr<CommandBuffer> &cmd_buffer,
    const std::vector<StaticMeshRenderData> &static_meshe_datas) {

  cmd_buffer->bindPipeline(pipeline_);
  for (auto &data : static_meshe_datas) {
    // descriptor set layout compatibility: https://registry.khronos.org/vulkan/specs/1.3-extensions/html/vkspec.html#descriptorsets-compatibility
    cmd_buffer->bindDescriptorSets(pipeline_, {data.material_descriptor_set}, {}, 1);
    cmd_buffer->pushConstants(pipeline_, VK_SHADER_STAGE_VERTEX_BIT, 0,
                              sizeof(TransformPCO), &data.transform_pco);
    cmd_buffer->bindVertexBuffers({data.vertex_buffer}, {0}, 0);
    cmd_buffer->bindIndexBuffer(data.index_buffer, 0, VK_INDEX_TYPE_UINT32);
    for (auto i = 0; i < data.index_counts.size(); ++i) {
      cmd_buffer->drawIndexed(data.index_counts[i], 1, data.first_index[i], 0,
                              0);
    }
  }
}

} // namespace mango