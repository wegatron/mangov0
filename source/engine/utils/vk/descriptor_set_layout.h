#pragma once

#include <engine/utils/vk/shader_module.h>
#include <engine/utils/vk/vk_driver.h>
#include <memory>
#include <vector>

namespace mango {
class DescriptorSetLayout final {
public:
  DescriptorSetLayout(const std::shared_ptr<VkDriver> &driver,
                      const ShaderResource *resources,
                      const uint32_t resource_size);

  DescriptorSetLayout(const DescriptorSetLayout &) = delete;
  DescriptorSetLayout(DescriptorSetLayout &&) = delete;

  DescriptorSetLayout &operator=(const DescriptorSetLayout &) = delete;
  DescriptorSetLayout &operator=(DescriptorSetLayout &&) = delete;

  ~DescriptorSetLayout();

  VkDescriptorSetLayout getHandle() const noexcept { return handle_; }

private:
  std::shared_ptr<VkDriver> driver_;
  VkDescriptorSetLayout handle_{VK_NULL_HANDLE};
  std::vector<VkDescriptorBindingFlagsEXT> binding_flags_;
  std::vector<VkDescriptorSetLayoutBinding> bindings_;
};
} // namespace mango