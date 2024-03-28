#pragma once

#include <engine/utils/vk/descriptor_set_layout.h>
#include <engine/utils/vk/vk_driver.h>
#include <memory>

namespace mango {
class DescriptorSet;
class DescriptorPool final {
public:
  DescriptorPool(const std::shared_ptr<VkDriver> &driver,
                 const VkDescriptorPoolCreateFlags flags,
                 const VkDescriptorPoolSize *pool_sizes,
                 const uint32_t pool_size_cnt, const uint32_t max_sets);

  DescriptorPool(const DescriptorPool &) = delete;
  DescriptorPool &operator=(const DescriptorPool &) = delete;

  ~DescriptorPool();

  VkDescriptorPool getHandle() const { return descriptor_pool_; }

  void reset();

  std::shared_ptr<DescriptorSet>
  requestDescriptorSet(const DescriptorSetLayout &layout);

  VkDescriptorPoolCreateFlags getFlags() const { return flags_; }

private:
  std::shared_ptr<VkDriver> driver_;
  VkDescriptorPoolCreateFlags flags_;
  VkDescriptorPool descriptor_pool_{VK_NULL_HANDLE};
  std::vector<std::shared_ptr<DescriptorSet>> descriptor_sets_;
};

class DescriptorSet {
public:
  DescriptorSet(const DescriptorSet &) = delete;
  DescriptorSet &operator=(const DescriptorSet &) = delete;

  ~DescriptorSet();

  VkDescriptorSet getHandle() const { return descriptor_set_; }

private:
  DescriptorSet(const std::shared_ptr<VkDriver> &driver, DescriptorPool &pool,
                const DescriptorSetLayout &layout);
  std::shared_ptr<VkDriver> driver_;
  bool free_able_{false};
  VkDescriptorPool descriptor_pool_{VK_NULL_HANDLE};
  VkDescriptorSet descriptor_set_{VK_NULL_HANDLE};

  friend class DescriptorPool;
};
} // namespace mango