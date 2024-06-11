
#include <engine/utils/base/error.h>
#include <engine/utils/vk/buffer.h>
#include <engine/utils/vk/commands.h>
#include <engine/utils/vk/stage_pool.h>
#include <engine/utils/vk/vk_driver.h>

namespace mango {
Buffer::Buffer(const std::shared_ptr<VkDriver> &driver,
               VkDeviceSize size,
               VkBufferUsageFlags buffer_usage,
               VkBufferCreateFlags flags,               
               VmaAllocationCreateFlags allocation_flags,
               VmaMemoryUsage memory_usage)
    : driver_(driver), flags_(flags), size_(size), buffer_usage_(buffer_usage),
      allocation_flags_(allocation_flags), memory_usage_(memory_usage) {
  persistent_ = (allocation_flags & VMA_ALLOCATION_CREATE_MAPPED_BIT) != 0;

  VkBufferCreateInfo buffer_create_info = {
      VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, // VkStructureType        sType;
      nullptr,                              // const void*            pNext;
      flags_,                               // VkBufferCreateFlags    flags;
      size_,                                // VkDeviceSize           size;
      buffer_usage_,                        // VkBufferUsageFlags     usage;
      VK_SHARING_MODE_EXCLUSIVE, // VkSharingMode          sharingMode;
      0,      // uint32_t               queueFamilyIndexCount;
      nullptr // const uint32_t*        pQueueFamilyIndices;
  };

  VmaAllocationCreateInfo alloc_create_info = {
      allocation_flags_, // VmaAllocationCreateFlags  flags;
      memory_usage_,     // VmaMemoryUsage            usage;
  };

  VmaAllocationInfo allocation_info{};
  auto result = vmaCreateBuffer(driver_->getAllocator(), &buffer_create_info,
                                &alloc_create_info, &buffer_, &allocation_,
                                &allocation_info);
  if (result != VK_SUCCESS) {
    throw VulkanException(result, "failed to create buffer!");
  }

  if (persistent_) {
    mapped_data_ = static_cast<std::byte *>(allocation_info.pMappedData);
    mapped_ = true;
  }
}

Buffer::~Buffer() {
  unmap();
  vmaDestroyBuffer(driver_->getAllocator(), buffer_, allocation_);
}

void Buffer::update(const void *data, size_t size, size_t offset) {
  if (persistent_) {
    memcpy(mapped_data_ + offset, data, size);
    // std::copy(data, data + size, mapped_data_ + offset);
    flush();
  } else {
    map();
    memcpy(mapped_data_ + offset, data, size);
    // std::copy(data, data + size, mapped_data_ + offset);
    flush();
    unmap();
  }
}

void Buffer::updateByStaging(void *data, size_t size, size_t offset,
                             const std::shared_ptr<CommandBuffer> &cmd_buf) {
  auto stage_pool = driver_->getStagePool();
  auto stage = stage_pool->acquireStage(size);
  // cpu data to stage
  void *mapped;
  vmaMapMemory(driver_->getAllocator(), stage->memory, &mapped);
  memcpy(mapped, data, size);
  vmaUnmapMemory(driver_->getAllocator(), stage->memory);
  vmaFlushAllocation(driver_->getAllocator(), stage->memory, 0, size);

  // stage buffer to gpu buffer
  VkBufferCopy region{.srcOffset = 0, .dstOffset = offset, .size = size};

  vkCmdCopyBuffer(cmd_buf->getHandle(), stage->buffer, buffer_, 1, &region);
}

void Buffer::flush() {
  // called after writing to a mapped memory for memory types that are not
  // HOST_COHERENT Unmap operation doesn't do that automatically.
  vmaFlushAllocation(driver_->getAllocator(), allocation_, 0, size_);
}

void Buffer::map() {
  if (!mapped_) {
    auto result = vmaMapMemory(driver_->getAllocator(), allocation_,
                               reinterpret_cast<void **>(&mapped_data_));
    if (result != VK_SUCCESS) {
      throw VulkanException(result, "failed to map buffer memory!");
    }
    mapped_ = true;
  }
}

void Buffer::unmap() {
  if (mapped_ && !persistent_) {
    vmaUnmapMemory(driver_->getAllocator(), allocation_);
    mapped_ = false;
  }
}

} // namespace mango