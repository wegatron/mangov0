#pragma once

#include <engine/utils/vk/descriptor_set_layout.h>
#include <engine/utils/vk/shader_module.h>
#include <engine/utils/vk/vk_driver.h>
#include <map>
#include <volk.h>

namespace mango {
class PipelineLayout final {
public:
  PipelineLayout(
      const std::shared_ptr<VkDriver> &driver,
      const std::vector<std::shared_ptr<ShaderModule>> &shader_modules);

  PipelineLayout(const PipelineLayout &) = delete;
  PipelineLayout &operator=(const PipelineLayout &) = delete;

  ~PipelineLayout();

  PipelineLayout(PipelineLayout &&) = delete;
  PipelineLayout &operator=(PipelineLayout &&) = delete;

  VkPipelineLayout getHandle() const { return handle_; }

  const DescriptorSetLayout &
  getDescriptorSetLayout(const uint32_t set_index) const;

private:
  std::shared_ptr<VkDriver> driver_;

  // all resources statics
  std::map<std::string, ShaderResource> resources_;

  // A map of each set and the resources it owns used by the pipeline layout
  std::map<uint32_t, std::vector<ShaderResource>> set_resources_;

  std::vector<std::shared_ptr<DescriptorSetLayout>> descriptor_set_layouts_;

  VkPipelineLayout handle_{VK_NULL_HANDLE};
};
} // namespace mango