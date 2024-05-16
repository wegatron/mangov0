#include "command_buffer_mgr.h"
#include <engine/utils/vk/commands.h>

namespace mango {

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

void ThreadLocalCommandBufferManager::commitExecutableCommandBuffers() {
  if (cur_primary_command_buffer_ != nullptr) {
    cur_primary_command_buffer_->end();
    executable_command_buffers_.push_back(
        cur_primary_command_buffer_->getHandle());
    cur_primary_command_buffer_ = nullptr;
  }

  if (executable_command_buffers_.empty())
    return;

  // commit command buffers
}
} // namespace mango