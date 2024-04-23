#include "material_normal_vis.h"
#include <engine/functional/global/app_context.h>
#include <engine/utils/vk/buffer.h>
#include <engine/utils/vk/image.h>
#include <engine/utils/vk/pipeline.h>
#include <engine/utils/vk/resource_cache.h>
#include <engine/utils/vk/sampler.h>

namespace mango {
NormalVisMaterial::NormalVisMaterial() {
  // all candidate inputs
  ubo_info_ = MaterialUboInfo{.set = MATERIAL_SET_INDEX,
                              .binding = 0,
                              .size = 32,
                              .data = std::vector<std::byte>(32, std::byte{0}),
                              .params{{.stride = 0,
                                       .ub_offset = 0,
                                       .tinfo = typeid(Eigen::Vector4f),
                                       .name = "pbr_mat.base_color"},
                                      {.stride = 0,
                                       .ub_offset = sizeof(float) * 4,
                                       .tinfo = typeid(float),
                                       .name = "pbr_mat.metallic"},
                                      {.stride = 0,
                                       .ub_offset = sizeof(float) * 5,
                                       .tinfo = typeid(float),
                                       .name = "pbr_mat.roughness"},
                                      {.stride = 0,
                                       .ub_offset = sizeof(float) * 6,
                                       .tinfo = typeid(float),
                                       .name = "pbr_mat.specular"}}};
  texture_params_ = {{.set = MATERIAL_SET_INDEX,
                      .binding = BASE_COLOR_TEXTURE_INDEX + 1,
                      .index = 0,
                      .name = BASE_COLOR_TEXTURE_NAME,
                      .def = "HAS_BASE_COLOR_TEXTURE",
                      .img_view = nullptr,
                      .dirty = false},
                     {.set = MATERIAL_SET_INDEX,
                      .binding = METALLIC_TEXTURE_INDEX + 1,
                      .index = 0,
                      .name = METALLIC_TEXTURE_NAME,
                      .def = "HAS_METALLIC_TEXTURE",
                      .img_view = nullptr,
                      .dirty = false},
                     {.set = MATERIAL_SET_INDEX,
                      .binding = +1,
                      .index = 0,
                      .name = ROUGHNESS_TEXTURE_NAME,
                      .def = "HAS_ROUGHNESS_TEXTURE",
                      .img_view = nullptr,
                      .dirty = false},
                     {.set = MATERIAL_SET_INDEX,
                      .binding = METALLIC_ROUGHNESS_TEXTURE_INDEX + 1,
                      .index = 0,
                      .name = METALLIC_ROUGHNESS_TEXTURE_NAME,
                      .def = "HAS_METALLIC_ROUGHNESS_TEXTURE",
                      .img_view = nullptr,
                      .dirty = false},
                     {.set = MATERIAL_SET_INDEX,
                      .binding = SPECULAR_TEXTURE_INDEX + 1,
                      .index = 0,
                      .name = SPECULAR_TEXTURE_NAME,
                      .def = "HAS_SPECULAR_TEXTURE",
                      .img_view = nullptr,
                      .dirty = false},
                     {.set = MATERIAL_SET_INDEX,
                      .binding = NORMAL_TEXTURE_INDEX + 1,
                      .index = 0,
                      .name = NORMAL_TEXTURE_NAME,
                      .def = "HAS_NORMAL_MAP",
                      .img_view = nullptr,
                      .dirty = false}};
  assert(texture_params_.size() == MAT_TEXTURE_NUM_COUNT);
}

void NormalVisMaterial::compile() {
  ShaderVariant variant;

  material_type_id_ = PBR_MATERIAL;

  std::vector<ShaderResource> sr;
  sr.reserve(texture_params_.size() + 1);
  sr.emplace_back(ShaderResource{.stages = VK_SHADER_STAGE_FRAGMENT_BIT,
                                 .type = ShaderResourceType::BufferUniform,
                                 .mode = ShaderResourceMode::Static,
                                 .set = MATERIAL_SET_INDEX,
                                 .binding = 0,
                                 .array_size = 1,
                                 .size = ubo_info_.size,
                                 .name = "pbr_mat"});

  // shader variance
  for (auto &tp : texture_params_) {
    if (tp.img_view != nullptr) {
      variant.addDefine(tp.def);
      material_type_id_ |= (1u << tp.binding);
      sr.emplace_back(ShaderResource{.stages = VK_SHADER_STAGE_FRAGMENT_BIT,
                                     .type = ShaderResourceType::ImageSampler,
                                     .mode = ShaderResourceMode::Static,
                                     .set = MATERIAL_SET_INDEX,
                                     .binding = tp.binding,
                                     .array_size = 1,
                                     .name = tp.name});
    }
  }
  desc_set_layout_ = std::make_unique<DescriptorSetLayout>(
      getDefaultAppContext().driver, MATERIAL_SET_INDEX, sr.data(), sr.size());

  vs_ = std::make_shared<ShaderModule>(variant);
  vs_->load("shaders/normal_visualize.vert");

  gs_ = std::make_shared<ShaderModule>(variant);
  gs_->load("shaders/normal_visualize.geom");

  fs_ = std::make_shared<ShaderModule>(variant);
  fs_->load("shaders/normal_visualize.frag");
}

std::shared_ptr<MatParamsSet>
NormalVisMaterial::createMatParamsSet(const std::shared_ptr<VkDriver> &driver,
                                      DescriptorPool &desc_pool) {
  auto ret = std::make_shared<MatParamsSet>();
  ret->mat_type_id = material_type_id_;
  // create uniform buffer
  ret->ubo = std::make_unique<Buffer>(
      driver, 0, ubo_info_.size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      VMA_ALLOCATION_CREATE_MAPPED_BIT |
          VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
      VMA_MEMORY_USAGE_AUTO_PREFER_HOST);

  // create descriptor set
  ret->desc_set = desc_pool.requestDescriptorSet(*desc_set_layout_);
  // update descriptor set
  VkDescriptorBufferInfo desc_buffer_info{
      .buffer = ret->ubo->getHandle(),
      .offset = 0,
      .range = ubo_info_.size,
  };
  driver->update(
      {VkWriteDescriptorSet{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                            .dstSet = ret->desc_set->getHandle(),
                            .dstBinding = 0,
                            .descriptorCount = 1,
                            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                            .pBufferInfo = &desc_buffer_info}});

  return ret;
}

void NormalVisMaterial::setPipelineState(GPipelineState &pipeline_state) {
  VertexInputState vertex_input_state{
      {// bindings, 3 float pos + 3 float normal + 2 float uv
       {0, 8 * sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX}},
      {                                                       // attribute
       {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0},                 // 3floats pos
       {1, 0, VK_FORMAT_R32G32B32_SFLOAT, 3 * sizeof(float)}, // 3floats normal
       {2, 0, VK_FORMAT_R32G32_SFLOAT, 6 * sizeof(float)}}};  // 2 floats uv
  pipeline_state.setVertexInputState(vertex_input_state);
  pipeline_state.setInputAssemblyState(
      {VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, false});
  RasterizationState rasterize{.depth_clamp_enable = false,
                               .rasterizer_discard_enable = false,
                               .polygon_mode = VK_POLYGON_MODE_FILL,
                               //.cull_mode = VK_CULL_MODE_BACK_BIT,
                               .cull_mode = VK_CULL_MODE_NONE,
                               .front_face = VK_FRONT_FACE_COUNTER_CLOCKWISE,
                               .depth_bias_enable = false};
  pipeline_state.setRasterizationState(rasterize);
  pipeline_state.setShaders({vs_, fs_});
  pipeline_state.setMultisampleState(
      {VK_SAMPLE_COUNT_1_BIT, false, 0.0f, 0xFFFFFFFF, false, false});
  // default depth stencil state, depth test enable, depth write enable, depth
  ColorBlendState color_blend_st{
      .attachments = {{
          .blendEnable = VK_FALSE,
          .colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                            VK_COLOR_COMPONENT_G_BIT |
                            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
      }}};
  pipeline_state.setColorBlendState(color_blend_st);

  pipeline_state.setSubpassIndex(0);
}

} // namespace mango