#pragma once

#include <engine/utils/vk/descriptor_set.h>


namespace mango {
class VkDriver;
class Buffer;
class ResourceBindingMgr final {
public:
  ResourceBindingMgr(const std::shared_ptr<VkDriver> &driver);
  ~ResourceBindingMgr();

  ResourceBindingMgr(const ResourceBindingMgr &) = delete;
  ResourceBindingMgr &operator=(const ResourceBindingMgr &) = delete;
  ResourceBindingMgr(ResourceBindingMgr &&) = delete;

  std::tuple<std::shared_ptr<DescriptorSet>, std::shared_ptr<Buffer>, uint32_t> requestStandardMaterial();

  std::pair<std::shared_ptr<DescriptorSet>, std::shared_ptr<Buffer>> requestLighting();

private:
  std::shared_ptr<VkDriver> driver_;
  
  // used for static standard material
  std::unique_ptr<DescriptorPool> desc_pool_;
  DescriptorSetLayout standard_material_layout_;
  std::shared_ptr<Buffer> umaterial_buffer_; //!< support 100 materials
  std::shared_ptr<Buffer> lighting_buffer_; //!< support lighting ubo
  std::shared_ptr<DescriptorSet> lighting_desc_set_;
  uint32_t standard_material_align_size_{0};
  uint32_t offset_{0};

};
} // namespace mango