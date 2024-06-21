#include <engine/functional/global/resource_binding_mgr.h>
#include <engine/utils/vk/buffer.h>
#include <engine/utils/vk/vk_driver.h>
#include <shaders/include/shader_structs.h>

namespace mango {
constexpr uint32_t kMaxMaterialCount = 100;
ResourceBindingMgr::ResourceBindingMgr(const std::shared_ptr<VkDriver> &driver) : driver_(driver) {
  //driver_ = driver;
  VkDescriptorPoolSize pool_sizes[] = {
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, kMaxMaterialCount},
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, kMaxMaterialCount * 4},
  };
  desc_pool_ = std::make_unique<DescriptorPool>(
      driver, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT, pool_sizes,
      sizeof(pool_sizes), kMaxMaterialCount);
  umaterial_buffer_ = std::make_shared<Buffer>(
      driver, sizeof(UMaterial) * kMaxMaterialCount,
      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      0, VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
      VMA_MEMORY_USAGE_AUTO_PREFER_HOST);
}

std::tuple<std::shared_ptr<DescriptorSet>, std::shared_ptr<Buffer>, uint32_t>
ResourceBindingMgr::requestStandardMaterial() {
  auto des_set = desc_pool_->requestDescriptorSet(standard_material_layout_);
  auto ret = std::make_tuple(des_set, umaterial_buffer_, offset_);
  offset_ += sizeof(UMaterial);
}
} // namespace mango