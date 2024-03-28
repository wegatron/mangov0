#pragma once

#include <memory>
#include <vk_mem_alloc.h>
#include <volk.h>

#include <engine/utils/vk/vk_driver.h>

namespace mango {
class CommandBuffer;
class StagePool;
class Buffer final {
public:
  Buffer(const std::shared_ptr<VkDriver> &driver, VkBufferCreateFlags flags,
         VkDeviceSize size, VkBufferUsageFlags buffer_usage,
         VmaAllocationCreateFlags allocation_flags,
         VmaMemoryUsage memory_usage);
  ~Buffer();

  Buffer(const Buffer &) = delete;
  Buffer(Buffer &&) = delete;
  Buffer &operator=(const Buffer &) = delete;

  VkBuffer getHandle() const { return buffer_; }

  void update(const void *data, size_t size, size_t offset = 0);

  void updateByStaging(void *data, size_t size, size_t offset,
                       const std::shared_ptr<StagePool> &stage_pool,
                       const std::shared_ptr<CommandBuffer> &cmd_buf);

private:
  void flush();

  void map();

  void unmap();

  std::shared_ptr<VkDriver> driver_;

  VkBuffer buffer_{VK_NULL_HANDLE};
  VmaAllocation allocation_{VK_NULL_HANDLE};

  bool mapped_{false};
  std::byte *mapped_data_{nullptr};

  bool persistent_;
  VkBufferCreateFlags flags_;
  VkDeviceSize size_;
  VkBufferUsageFlags buffer_usage_;

  VmaAllocationCreateFlags allocation_flags_;
  VmaMemoryUsage memory_usage_;
};
} // namespace mango