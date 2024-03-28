#include <engine/utils/base/error.h>
#include <engine/utils/vk/pipeline_layout.h>
#include <stdexcept>

namespace mango {
PipelineLayout::PipelineLayout(
    const std::shared_ptr<VkDriver> &driver,
    const std::vector<std::shared_ptr<ShaderModule>> &shader_modules)
    : driver_(driver) {
  // all resources statics
  for (const auto shader_module : shader_modules) {
    for (const auto &resource : shader_module->getResources()) {
      std::string key = resource.name;
      // input output may have same name
      if (resource.type == ShaderResourceType::Input ||
          resource.type == ShaderResourceType::Output) {
        key = std::to_string(resource.stages) + "_" + key;
      }

      auto itr = resources_.find(key);
      if (itr != resources_.end())
        itr->second.stages |= resource.stages;
      else
        resources_.emplace(key, resource);
    }
  }

  // resources for each set
  uint32_t max_set_index = 0;
  for (auto itr = resources_.begin(); itr != resources_.end(); ++itr) {
    auto &resource = itr->second;
    if (resource.type == ShaderResourceType::Input ||
        resource.type == ShaderResourceType::Output ||
        resource.type == ShaderResourceType::InputAttachment ||
        resource.type == ShaderResourceType::PushConstant)
      continue;
    auto &set_resources = set_resources_[resource.set];
    set_resources.emplace_back(resource);
    max_set_index = std::max(max_set_index, resource.set);
  }

  // create descriptor set layouts
  std::vector<VkDescriptorSetLayout> descriptor_set_layout_handles(
      max_set_index + 1);
  descriptor_set_layouts_.resize(max_set_index + 1);
  for (auto itr = set_resources_.begin(); itr != set_resources_.end(); ++itr) {
    auto set_index = itr->first;
    auto &set_resources = itr->second;
    descriptor_set_layouts_[set_index] = std::make_shared<DescriptorSetLayout>(
        driver, set_index, set_resources.data(), set_resources.size());
    descriptor_set_layout_handles[set_index] =
        descriptor_set_layouts_[set_index]->getHandle();
  }

  VkPipelineLayoutCreateInfo create_info{
      VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};

  create_info.setLayoutCount = descriptor_set_layout_handles.size();
  create_info.pSetLayouts = descriptor_set_layout_handles.data();
  create_info.pushConstantRangeCount = 0;
  create_info.pPushConstantRanges = nullptr;

  // Create the Vulkan pipeline layout handle
  auto result = vkCreatePipelineLayout(driver_->getDevice(), &create_info,
                                       nullptr, &handle_);
  VK_THROW_IF_ERROR(result, "Failed to create PipelineLayout.");
}

PipelineLayout::~PipelineLayout() {
  vkDestroyPipelineLayout(driver_->getDevice(), handle_, nullptr);
}

const DescriptorSetLayout &
PipelineLayout::getDescriptorSetLayout(const uint32_t set_index) const {
  auto itr = std::find_if(
      descriptor_set_layouts_.begin(), descriptor_set_layouts_.end(),
      [set_index](const std::shared_ptr<DescriptorSetLayout> &e) {
        return e->getSetIndex() == set_index;
      });
  assert(itr != descriptor_set_layouts_.end());
  return *(*itr);
}
} // namespace mango