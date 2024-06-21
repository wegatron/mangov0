#pragma once

#include <engine/utils/vk/descriptor_set.h>


namespace mango {
class VkDriver;
class Buffer;
class ResourceBindingMgr {
public:
  ResourceBindingMgr(const std::shared_ptr<VkDriver> &driver);
  ~ResourceBindingMgr() = default;

  ResourceBindingMgr(const ResourceBindingMgr &) = delete;
  ResourceBindingMgr &operator=(const ResourceBindingMgr &) = delete;
  ResourceBindingMgr(ResourceBindingMgr &&) = delete;

  std::tuple<std::shared_ptr<DescriptorSet>, std::shared_ptr<Buffer>, uint32_t> requestStandardMaterial();

private:
  std::shared_ptr<VkDriver> driver_;
  std::unique_ptr<DescriptorPool> desc_pool_;
  DescriptorSetLayout standard_material_layout_;
  std::shared_ptr<Buffer> umaterial_buffer_; //!< support 100 materials
  uint32_t offset_{0};
};
} // namespace mango