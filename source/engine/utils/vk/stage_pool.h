#pragma once

#include <engine/utils/vk/buffer.h>
#include <map>
#include <unordered_set>

namespace mango {
// Immutable POD representing a shared CPU-GPU staging area.
struct VulkanStage {
  VmaAllocation memory;
  VkBuffer buffer;
  uint32_t capacity;
  mutable uint64_t lastAccessed;
};

struct VulkanStageImage {
  VkFormat format;
  uint32_t width;
  uint32_t height;
  mutable uint64_t lastAccessed;
  VmaAllocation memory;
  VkImage image;
};

// Manages a pool of stages, periodically releasing stages that have been unused
// for a while. This class manages two types of host-mappable staging areas:
// buffer stages and image stages.
class StagePool final {
public:
  StagePool(const std::shared_ptr<VkDriver> &driver) : driver_(driver) {}

  ~StagePool() { reset(); }

  // Finds or creates a stage whose capacity is at least the given number of
  // bytes. The stage is automatically released back to the pool after
  // TIME_BEFORE_EVICTION frames.
  VulkanStage const *acquireStage(uint32_t numBytes);

  // Images have VK_IMAGE_LAYOUT_GENERAL and must not be transitioned to any
  // other layout
  VulkanStageImage const *acquireImage(VkFormat format, uint32_t width,
                                       uint32_t height,
                                       VkCommandBuffer cmd_buf);

  // Evicts old unused stages and bumps the current frame number.
  void gc() noexcept;

  // Destroys all unused stages and asserts that there are no stages currently
  // in use. This should be called while the context's VkDevice is still alive.
  void reset() noexcept;

private:
  std::shared_ptr<VkDriver> driver_;
  // Use an ordered multimap for quick (capacity => stage) lookups using
  // lower_bound().
  std::multimap<uint32_t, VulkanStage const *> free_stages_;

  // Simple unordered set for stashing a list of in-use stages that can be
  // reclaimed later.
  std::unordered_set<VulkanStage const *> used_stages_;

  std::unordered_set<VulkanStageImage const *> free_images_;
  std::unordered_set<VulkanStageImage const *> used_images_;

  // Store the current "time" (really just a frame count) and LRU eviction
  // parameters.
  uint64_t current_frame_{0};
};
} // namespace mango