#include <engine/utils/base/error.h>
#include <engine/utils/vk/pipeline_layout.h>
#include <engine/utils/vk/resource_cache.h>
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
  int32_t max_set_index = -1;
  for (auto itr = resources_.begin(); itr != resources_.end(); ++itr) {
    auto &resource = itr->second;
    if (resource.type == ShaderResourceType::Input ||
        resource.type == ShaderResourceType::Output ||
        resource.type == ShaderResourceType::InputAttachment ||
        resource.type == ShaderResourceType::PushConstant)
      continue;
    auto &set_resources = set_resources_[resource.set];
    set_resources.emplace_back(resource);
    max_set_index = std::max(max_set_index, static_cast<int32_t>(resource.set));
  }

  // push constant resources
  std::vector<VkPushConstantRange> push_const_ranges;
  for (const auto &shader_module : shader_modules) {
    for (const auto &resource : shader_module->getResources()) {
      if (resource.type == ShaderResourceType::PushConstant) {
        push_const_ranges.emplace_back(VkPushConstantRange{
            .stageFlags = resource.stages,
            .offset = resource.offset,
            .size = resource.size,
        });
      }
    }
  }

  auto rs_cache = g_engine.getResourceCache();
  // create descriptor set layouts
  auto num_ds_set = max_set_index + 1;
  std::vector<VkDescriptorSetLayout> descriptor_set_layout_handles(
      num_ds_set, VK_NULL_HANDLE);
  descriptor_set_layouts_.resize(num_ds_set, nullptr);
  for (auto set_index = 0; set_index < num_ds_set; ++set_index) {
    auto &set_resources = set_resources_[set_index]; // binding a empry descriptor set if no resources
    descriptor_set_layouts_[set_index] =
        rs_cache->requestDescriptorSetLayout(driver, set_resources);
    descriptor_set_layout_handles[set_index] =
        descriptor_set_layouts_[set_index]->getHandle();
  }

  VkPipelineLayoutCreateInfo create_info{
      VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};

  create_info.setLayoutCount = descriptor_set_layout_handles.size();
  create_info.pSetLayouts = descriptor_set_layout_handles.data();
  create_info.pushConstantRangeCount = push_const_ranges.size();
  create_info.pPushConstantRanges = push_const_ranges.data();

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
  assert(set_index < descriptor_set_layouts_.size());
  return *descriptor_set_layouts_[set_index];
}
} // namespace mango