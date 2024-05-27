#pragma once

#include <cassert>
#include <memory>
#include <thread>
#include <vector>
#include <volk.h>
#include <engine/utils/vk/vk_constants.h>

namespace mango {
class VkDriver;
class CommandBuffer;
class Fence;
class CommandPool;
class CommandQueue;
class Semaphore;

class ThreadLocalCommandBufferManager final {
public:
  ThreadLocalCommandBufferManager(const std::shared_ptr<VkDriver> &driver,
                                  uint32_t queue_family_index);

  ThreadLocalCommandBufferManager(const ThreadLocalCommandBufferManager &) =
      delete;

  ThreadLocalCommandBufferManager &
  operator=(const ThreadLocalCommandBufferManager &) = delete;

  ThreadLocalCommandBufferManager(ThreadLocalCommandBufferManager &&) = default;

  ~ThreadLocalCommandBufferManager() = default;

  std::thread::id getThreadId() const { return tid_; }

  void setThreadId(std::thread::id tid) { tid_ = tid; }

  /**
   * @brief request command buffer for record. the command buffer will be at
   * recording state, do not need to call begin.
   */
  std::shared_ptr<CommandBuffer>
  requestCommandBuffer(VkCommandBufferLevel level);

  /**
   * @brief push command buffer to candidate executable list. will call command
   * buffer's end() method.
   */
  void enqueueCommandBuffer(const std::shared_ptr<CommandBuffer> &cmd_buf);

  /**
   * @brief Get the Executable Command Buffers
   */
  std::vector<VkCommandBuffer> &&getExecutableCommandBuffers();

  VkResult commitExecutableCommandBuffers(const CommandQueue * queue, const std::shared_ptr<Semaphore> &signal_semaphore);

  std::shared_ptr<Fence> getCommandBufferAvailableFence() const {
    return command_buffer_available_fence_[*cur_frame_index_];
  }

  /**
   * @brief reset command pool
   */
  void resetCurFrameCommandPool();

private:
  std::thread::id tid_;
  const uint32_t *cur_frame_index_{nullptr};
  std::shared_ptr<CommandBuffer> cur_primary_command_buffer_;

  std::shared_ptr<Fence> command_buffer_available_fence_[MAX_FRAMES_IN_FLIGHT];
  std::shared_ptr<CommandPool> command_pool_[MAX_FRAMES_IN_FLIGHT];
  std::vector<VkCommandBuffer> executable_command_buffers_;
};
} // namespace mango