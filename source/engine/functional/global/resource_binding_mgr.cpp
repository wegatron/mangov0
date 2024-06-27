#include <engine/functional/global/resource_binding_mgr.h>
#include <engine/utils/vk/buffer.h>
#include <engine/utils/vk/vk_driver.h>
#include <shaders/include/shader_structs.h>

namespace mango {
constexpr uint32_t kMaxMaterialCount = 100;
ShaderResource kStandardMaterialResources[] = {
  {
    .stages = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
    .type = ShaderResourceType::BufferUniform,
    .mode = ShaderResourceMode::Static,
    .set = 1,
    .binding = 0,
    .name = "material_ubo"
  },
  {
    .stages = VK_SHADER_STAGE_FRAGMENT_BIT,
    .type = ShaderResourceType::ImageSampler,
    .mode = ShaderResourceMode::Static,
    .set = 1,
    .binding = 1,
    .name = "albedo_sampler"
  },
  {
    .stages = VK_SHADER_STAGE_FRAGMENT_BIT,
    .type = ShaderResourceType::ImageSampler,
    .mode = ShaderResourceMode::Static,
    .set = 1,
    .binding = 2,
    .name = "normal_sampler"
  },
  {
    .stages = VK_SHADER_STAGE_FRAGMENT_BIT,
    .type = ShaderResourceType::ImageSampler,
    .mode = ShaderResourceMode::Static,
    .set = 1,
    .binding = 3,
    .name = "emissive_sampler"    
  },
  {
    .stages = VK_SHADER_STAGE_FRAGMENT_BIT,
    .type = ShaderResourceType::ImageSampler,
    .mode = ShaderResourceMode::Static,
    .set = 1,
    .binding = 4,
    .name = "metallic_roughness_occlusion_sampler"
  }  
};
ResourceBindingMgr::ResourceBindingMgr(const std::shared_ptr<VkDriver> &driver)
    : driver_(driver), standard_material_layout_(driver, 1, kStandardMaterialResources,
                                sizeof(kStandardMaterialResources) /
                                    sizeof(ShaderResource)) {
  VkDescriptorPoolSize pool_sizes[] = {
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, kMaxMaterialCount},
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4*kMaxMaterialCount}, // albedo, normal, metallic_roughness_occlusion, emissive
  };
  desc_pool_ = std::make_unique<DescriptorPool>(
      driver, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT, pool_sizes,
      sizeof(pool_sizes)/sizeof(pool_sizes[0]), kMaxMaterialCount);
  auto min_ubo_align = driver->getMinUboAlignSize();
  standard_material_align_size_ = (sizeof(UMaterial) + min_ubo_align - 1) & ~(min_ubo_align - 1);
  umaterial_buffer_ = std::make_shared<Buffer>(
      driver, standard_material_align_size_ * kMaxMaterialCount,
      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      0, VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
      VMA_MEMORY_USAGE_AUTO_PREFER_HOST);
}

std::tuple<std::shared_ptr<DescriptorSet>, std::shared_ptr<Buffer>, uint32_t>
ResourceBindingMgr::requestStandardMaterial() {  
  auto standard_material_set = desc_pool_->requestDescriptorSet(standard_material_layout_);
  // // write descriptor set
  // VkDescriptorBufferInfo desc_buffer_info{
  //     .buffer = umaterial_buffer_->getHandle(),
  //     .offset = 0,
  //     .range = standard_material_align_size_ * kMaxMaterialCount
  // };  
  // driver_->update({VkWriteDescriptorSet{
  //   .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
  //   .dstSet = standard_material_set->getHandle(),
  //   .dstBinding = 0,
  //   .descriptorCount = 1,
  //   .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
  //   .pBufferInfo = &desc_buffer_info
  // }});  
  auto ret = std::make_tuple(standard_material_set, umaterial_buffer_, offset_);
  offset_ += standard_material_align_size_;
  return ret;
}

ResourceBindingMgr::~ResourceBindingMgr()
{
  umaterial_buffer_.reset();
  desc_pool_.reset();
}
} // namespace mango