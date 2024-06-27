#include <engine/utils/base/error.h>
#include <engine/utils/vk/descriptor_set.h>

namespace mango {
DescriptorPool::DescriptorPool(const std::shared_ptr<VkDriver> &driver,
                               const VkDescriptorPoolCreateFlags flags,
                               const VkDescriptorPoolSize *pool_sizes,
                               const uint32_t pool_sizes_cnt,
                               const uint32_t max_sets)
    : driver_(driver), flags_(flags) {
  VkDescriptorPoolCreateInfo pool_info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .flags = flags,
      .maxSets = max_sets,
      .poolSizeCount = pool_sizes_cnt,
      .pPoolSizes = pool_sizes};

  auto result = vkCreateDescriptorPool(driver_->getDevice(), &pool_info,
                                       nullptr, &descriptor_pool_);
  if (result != VK_SUCCESS) {
    throw VulkanException(result, "failed to create descriptor pool!");
  }
}

DescriptorPool::~DescriptorPool() {
  for (auto &descriptor_set : descriptor_sets_) {
    if (descriptor_set.use_count() > 1) {
      throw VulkanUseException(
          "descriptor pool reseting with descriptor set is still in use!");
    }
  }
  // all descriptor sets allocated from the pool are implicitly freed and become
  // invalid.
  vkDestroyDescriptorPool(driver_->getDevice(), descriptor_pool_, nullptr);
}

void DescriptorPool::reset() {
  for (auto &descriptor_set : descriptor_sets_) {
    if (descriptor_set.use_count() > 1) {
      throw VulkanUseException(
          "descriptor pool reseting with descriptor set is still in use!");
    }
  }

  // recycles all of the resources from all of the descriptor sets allocated
  // from the descriptor pool back to the descriptor pool, and the descriptor
  // sets are implicitly freed.
  vkResetDescriptorPool(driver_->getDevice(), descriptor_pool_, 0);
}

std::shared_ptr<DescriptorSet>
DescriptorPool::requestDescriptorSet(const DescriptorSetLayout &layout) {
  std::shared_ptr<DescriptorSet> ptr(new DescriptorSet(driver_, *this, layout));
  descriptor_sets_.emplace_back(ptr);
  return ptr;
}

DescriptorSet::DescriptorSet(const std::shared_ptr<VkDriver> &driver,
                             DescriptorPool &pool,
                             const DescriptorSetLayout &layout)
    : driver_(driver) {
  free_able_ =
      (pool.getFlags() & VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
  descriptor_pool_ = pool.getHandle();
  auto layout_handle = layout.getHandle();

  VkDescriptorSetAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  alloc_info.descriptorPool = descriptor_pool_;
  alloc_info.descriptorSetCount = 1;
  alloc_info.pSetLayouts = &layout_handle;

  auto result = vkAllocateDescriptorSets(driver_->getDevice(), &alloc_info,
                                         &descriptor_set_);
  if (result != VK_SUCCESS) {
    throw VulkanException(result, "failed to allocate descriptor set!");
  }
}

DescriptorSet::~DescriptorSet() {
  if (free_able_)
    vkFreeDescriptorSets(driver_->getDevice(), descriptor_pool_, 1,
                         &descriptor_set_);
}
} // namespace mango