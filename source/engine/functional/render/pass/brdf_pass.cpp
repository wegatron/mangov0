#include "brdf_pass.h"

#include <engine/functional/global/engine_context.h>
#include <engine/utils/vk/pipeline.h>
#include <engine/utils/vk/resource_cache.h>
#include <engine/utils/vk/shader_module.h>

namespace mango {
void BRDFPass::init() {
  // create pipeline state
  auto pipeline_state = std::make_unique<GPipelineState>();
  pipeline_state->setVertexInputState(VertexInputState{
      .vertex_binding_descriptions_ = {VkVertexInputBindingDescription{
          0, sizeof(float) * 8, // 3 for position, 3 for normal, 2 for uv
          VK_VERTEX_INPUT_RATE_VERTEX}},
      .vertex_attribute_descriptions_ =
          {VkVertexInputAttributeDescription{0, 0, VK_FORMAT_R32G32B32_SFLOAT,
                                             0}, // position
           VkVertexInputAttributeDescription{1, 0, VK_FORMAT_R32G32B32_SFLOAT,
                                             sizeof(float) * 3}, // normal
           VkVertexInputAttributeDescription{2, 0, VK_FORMAT_R32G32_SFLOAT,
                                             sizeof(float) * 6}}, // uv
  });

  pipeline_state->setInputAssemblyState(InputAssemblyState{
      .topology_ = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
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
      RasterizationState{.depth_clamp_enable_ = VK_FALSE,
                         .rasterizer_discard_enable_ = VK_FALSE,
                         .polygon_mode_ = VK_POLYGON_MODE_FILL,
                         .cull_mode = VK_CULL_MODE_BACK_BIT,
                         .front_face = VK_FRONT_FACE_COUNTER_CLOCKWISE,
                         .depth_bias_enable = VK_FALSE});

  pipeline_state->setMultisampleState(
      MultisampleState{.rasterization_samples = VK_SAMPLE_COUNT_1_BIT,
                       .sample_shading_enable = VK_FALSE,
                       .min_sample_shading = 1.0f,
                       .sample_mask = 0xFFFFFFFF,
                       .alpha_to_coverage_enable = VK_FALSE,
                       .alpha_to_one_enable = VK_FALSE});
  pipeline_state->setDepthStencilState(
      DepthStencilState{.depth_test_enable = VK_TRUE,
                        .depth_write_enable = VK_TRUE,
                        .depth_compare_op = VK_COMPARE_OP_LESS,
                        .depth_bounds_test_enable = VK_FALSE,
                        .stencil_test_enable = VK_FALSE,
                        .front = VkStencilOpState{},
                        .back = VkStencilOpState{},
                        .min_depth_bounds = 0.0f,
                        .max_depth_bounds = 1.0f});
  auto driver = g_engine.getDriver();
  auto resource_cache = g_engine.getResourceCache();
  auto render_pass = resource_cache->requestRenderPass(
      driver,
      {Attachment{.format = VK_FORMAT_R8G8B8_SRGB},
       Attachment{.format = VK_FORMAT_D24_UNORM_S8_UINT}},
      {LoadStoreInfo{}, LoadStoreInfo{}},
      {SubpassInfo{}}); // only format is used
  pipeline_ = std::make_shared<GraphicsPipeline>(
      driver, resource_cache, render_pass, std::move(pipeline_state));
  // using default color blend state: not blend

  // create descriptor set
  // create frame buffer
}
} // namespace mango