#include "command_buffer_mgr.h"
#include <engine/utils/vk/commands.h>
#include <engine/utils/vk/syncs.h>

namespace mango {

ThreadLocalCommandBufferManager::ThreadLocalCommandBufferManager(const std::shared_ptr<VkDriver> &driver,
                                  uint32_t queue_family_index)
{  
  tid_ = std::this_thread::get_id();
  cur_frame_index_ = &driver->getCurFrameIndexRef();
  for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
    command_pool_[i] = std::make_shared<CommandPool>(driver, queue_family_index, CommandPool::CmbResetMode::ResetPool);
    command_buffer_available_fence_[i] = std::make_shared<Fence>(driver, true);
  }
}                                  

std::shared_ptr<CommandBuffer>
ThreadLocalCommandBufferManager::requestCommandBuffer(
    VkCommandBufferLevel level) {
  if (level == VK_COMMAND_BUFFER_LEVEL_PRIMARY) {
    if (cur_primary_command_buffer_ == nullptr) {
      cur_primary_command_buffer_ =
          command_pool_[*cur_frame_index_]->requestCommandBuffer(level);
      cur_primary_command_buffer_->begin(
          VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    }
    return cur_primary_command_buffer_;
  } else {
    assert(false && "not support secondary command buffer");
    // if (cur_secondary_command_buffer_ == nullptr) {
    //     cur_secondary_command_buffer_ =
    //     command_pool_[*cur_frame_index_]->requestCommandBuffer(level);
    // }
    // return cur_secondary_command_buffer_;
  }
}

void ThreadLocalCommandBufferManager::enqueueCommandBuffer(
    const std::shared_ptr<CommandBuffer> &cmd_buf) {
  assert(cmd_buf == cur_primary_command_buffer_);
  cur_primary_command_buffer_->end();
  executable_command_buffers_.push_back(
      cur_primary_command_buffer_->getHandle());
  cur_primary_command_buffer_ = nullptr;
}

std::vector<VkCommandBuffer> &&
ThreadLocalCommandBufferManager::getExecutableCommandBuffers() {
  if (cur_primary_command_buffer_ != nullptr) {
    cur_primary_command_buffer_->end();
    executable_command_buffers_.push_back(
        cur_primary_command_buffer_->getHandle());
    cur_primary_command_buffer_ = nullptr;
  }
  return std::move(executable_command_buffers_);
}

void ThreadLocalCommandBufferManager::resetCurFrameCommandPool() {
  command_pool_[*cur_frame_index_]->reset();
}

VkResult ThreadLocalCommandBufferManager::commitExecutableCommandBuffers(
    const CommandQueue *queue, const std::shared_ptr<Semaphore> &signal_semaphore) {
  if (cur_primary_command_buffer_ != nullptr) {
    cur_primary_command_buffer_->end();
    executable_command_buffers_.push_back(
        cur_primary_command_buffer_->getHandle());
    cur_primary_command_buffer_ = nullptr;
  }

  if (executable_command_buffers_.empty())
    return VK_SUCCESS;
  auto semaphore_handle = signal_semaphore != VK_NULL_HANDLE ? signal_semaphore->getHandle() : VK_NULL_HANDLE;
  // commit command buffers
  VkSubmitInfo submit_info{VK_STRUCTURE_TYPE_SUBMIT_INFO};
  submit_info.commandBufferCount = executable_command_buffers_.size();
  submit_info.pCommandBuffers = executable_command_buffers_.data();
  submit_info.waitSemaphoreCount = 0;
  submit_info.pWaitSemaphores = VK_NULL_HANDLE;
  submit_info.pWaitDstStageMask = VK_NULL_HANDLE;
  submit_info.signalSemaphoreCount = signal_semaphore != nullptr ? 1 : 0;
  submit_info.pSignalSemaphores = &semaphore_handle;
  auto fence = command_buffer_available_fence_[*cur_frame_index_];
  fence->reset();
  auto fence_handle = fence->getHandle();
  auto result = queue->submit({submit_info}, fence_handle);
  executable_command_buffers_.clear();
  return result;
}
} // namespace mango