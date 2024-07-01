#include <engine/utils/base/error.h>
#include <engine/utils/vk/buffer.h>
#include <engine/utils/vk/commands.h>
#include <engine/utils/vk/descriptor_set.h>
#include <engine/utils/vk/framebuffer.h>
#include <engine/utils/vk/image.h>
#include <engine/utils/vk/pipeline.h>
#include <iostream>

namespace mango {

VkResult CommandQueue::submit(const std::vector<VkSubmitInfo> &submit_infos,
                              VkFence fence) const {
  return vkQueueSubmit(handle_, submit_infos.size(), submit_infos.data(),
                       fence);
}

VkResult CommandQueue::submit(const std::shared_ptr<CommandBuffer> &cmd_buf,
                              VkFence fence) const {
  // submit
  auto cmd_buf_handle = cmd_buf->getHandle();
  VkSubmitInfo submit_info{VK_STRUCTURE_TYPE_SUBMIT_INFO};
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &cmd_buf_handle;
  submit_info.waitSemaphoreCount = 0;
  submit_info.pWaitSemaphores = NULL;
  submit_info.pWaitDstStageMask = NULL;
  submit_info.signalSemaphoreCount = 0;
  submit_info.pSignalSemaphores = NULL;
  return vkQueueSubmit(handle_, 1, &submit_info, fence);
}

VkResult CommandQueue::present(const VkPresentInfoKHR &present_info) const {
  if (!can_present_) {
    return VK_ERROR_INCOMPATIBLE_DISPLAY_KHR;
  }

  return vkQueuePresentKHR(handle_, &present_info);
}

VkResult CommandQueue::waitIdle() const { return vkQueueWaitIdle(handle_); }

CommandPool::CommandPool(const std::shared_ptr<VkDriver> &driver,
                         uint32_t queue_family_index, CmbResetMode reset_mode)
    : driver_(driver), reset_mode_(reset_mode) {
  VkCommandPoolCreateFlags flags[] = {
      VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      VK_COMMAND_POOL_CREATE_TRANSIENT_BIT};

  VkCommandPoolCreateInfo pool_info = {};
  pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_info.queueFamilyIndex = queue_family_index;
  pool_info.flags = flags[static_cast<int>(reset_mode)];

  auto result = vkCreateCommandPool(driver->getDevice(), &pool_info, nullptr,
                                    &command_pool_);
  if (result != VK_SUCCESS) {
    throw VulkanException(result, "failed to create command pool!");
  }
}

CommandPool::~CommandPool() {
  for (auto &command_buffer : primary_command_buffers_) {
    if (command_buffer.use_count() != 1) {
      throw VulkanException(
          VK_RESULT_MAX_ENUM,
          "command pool destroy with command buffer is still in use!");
    }
  }

  for (auto &command_buffer : secondary_command_buffers_) {
    if (command_buffer.use_count() != 1) {
      throw VulkanException(
          VK_RESULT_MAX_ENUM,
          "command pool destroy with command buffer is still in use!");
    }
  }

  primary_command_buffers_.clear();
  secondary_command_buffers_.clear();

  vkDestroyCommandPool(driver_->getDevice(), command_pool_, nullptr);
}

void CommandPool::reset() {
  switch (reset_mode_) {
  case CmbResetMode::ResetPool: {
    vkResetCommandPool(driver_->getDevice(), command_pool_, 0);
    break;
  }
  case CmbResetMode::ResetIndividually: {
    for (auto cmb : primary_command_buffers_) {
      if (cmb.use_count() == 1)
        cmb->reset();
    }

    for (auto cmb : secondary_command_buffers_) {
      if (cmb.use_count() == 1)
        cmb->reset();
    }
    break;
  }
  case CmbResetMode::AlwaysAllocate: {
    primary_command_buffers_.clear();
    secondary_command_buffers_.clear();
    break;
  }
  default:
    break;
  }

  active_primary_command_buffer_count_ = 0;
  active_secondary_command_buffer_count_ = 0;
}

std::shared_ptr<CommandBuffer>
CommandPool::requestCommandBuffer(VkCommandBufferLevel level) {
  if (reset_mode_ == CmbResetMode::ResetPool) {
    if (level == VK_COMMAND_BUFFER_LEVEL_PRIMARY) {
      if (active_primary_command_buffer_count_ <
          primary_command_buffers_.size()) {
        return primary_command_buffers_.at(
            active_primary_command_buffer_count_++);
      }
      primary_command_buffers_.emplace_back(
          new CommandBuffer(driver_, *this, level));
      active_primary_command_buffer_count_++;
      return primary_command_buffers_.back();
    }

    if (active_secondary_command_buffer_count_ <
        secondary_command_buffers_.size()) {
      return secondary_command_buffers_.at(
          active_secondary_command_buffer_count_++);
    }

    secondary_command_buffers_.emplace_back(
        new CommandBuffer(driver_, *this, level));
    active_secondary_command_buffer_count_++;

    return secondary_command_buffers_.back();
  } else if(reset_mode_ == CmbResetMode::ResetIndividually) {
    if (level == VK_COMMAND_BUFFER_LEVEL_PRIMARY) {
      for (auto &command_buffer : primary_command_buffers_) {
        if (command_buffer.use_count() == 1) {
          command_buffer->reset();
          return command_buffer;
        }
      }
      primary_command_buffers_.emplace_back(new CommandBuffer(driver_, *this, level));
      return primary_command_buffers_.back();
    } else {
      for (auto &command_buffer : secondary_command_buffers_) {
        if (command_buffer.use_count() == 1) {
          command_buffer->reset();
          return command_buffer;
        }
      }
      secondary_command_buffers_.emplace_back(
        new CommandBuffer(driver_, *this, level));      
      return secondary_command_buffers_.back();
    }
  } else {
    auto cmd_buffer = new CommandBuffer(driver_, *this, level);
    if (level == VK_COMMAND_BUFFER_LEVEL_PRIMARY) {
      primary_command_buffers_.emplace_back(cmd_buffer);
      return primary_command_buffers_.back();
    }
    secondary_command_buffers_.emplace_back(cmd_buffer);
    return secondary_command_buffers_.back();
  }
  return nullptr;
}

CommandBuffer::CommandBuffer(const std::shared_ptr<VkDriver> &driver,
                             CommandPool &command_pool,
                             VkCommandBufferLevel level)
    : driver_(driver), command_pool_(command_pool.getHandle()) {
#ifndef NDEBUG
  resetable_ = (command_pool.getResetMode() ==
                CommandPool::CmbResetMode::ResetIndividually);
#endif

  VkCommandBufferAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.commandPool = command_pool.getHandle();
  alloc_info.level = level;
  alloc_info.commandBufferCount = 1;

  auto result = vkAllocateCommandBuffers(driver_->getDevice(), &alloc_info,
                                         &command_buffer_);
  if (result != VK_SUCCESS) {
    throw VulkanException(result, "failed to allocate command buffers!");
  }
}

CommandBuffer::~CommandBuffer() {
  if (command_buffer_ != VK_NULL_HANDLE) {
    vkFreeCommandBuffers(driver_->getDevice(), command_pool_, 1,
                         &command_buffer_);
  }
}

void CommandBuffer::reset() {
#ifndef NDEBUG
  if (!resetable_) {
    throw VulkanException(VK_RESULT_MAX_ENUM,
                          "command buffer is not resetable!");
  }
#endif

  vkResetCommandBuffer(command_buffer_,
                       VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
}

void CommandBuffer::begin(VkCommandBufferUsageFlags flags) {
  VkCommandBufferBeginInfo begin_info = {};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = flags;
  begin_info.pInheritanceInfo = nullptr;

  auto result = vkBeginCommandBuffer(command_buffer_, &begin_info);
  VK_THROW_IF_ERROR(result, "failed to begin recording command buffer!");
}

void CommandBuffer::end() {
  auto result = vkEndCommandBuffer(command_buffer_);
  VK_THROW_IF_ERROR(result, "failed to record command buffer!");
}

void CommandBuffer::beginRenderPass(
    const std::shared_ptr<RenderPass> &render_pass,
    const std::shared_ptr<FrameBuffer> &frame_buffer) {
  VkClearValue clear_values[2]; // 与render pass load store clear attachment对应
  clear_values[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
  clear_values[1].depthStencil = {1.0f, 0};

  VkRenderPassBeginInfo render_pass_begin_info = {};
  render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  render_pass_begin_info.renderPass = render_pass->getHandle();
  render_pass_begin_info.framebuffer = frame_buffer->getHandle();
  render_pass_begin_info.renderArea.offset = {0, 0};
  render_pass_begin_info.renderArea.extent.width = frame_buffer->getWidth();
  render_pass_begin_info.renderArea.extent.height = frame_buffer->getHeight();
  render_pass_begin_info.clearValueCount = 2;
  render_pass_begin_info.pClearValues = clear_values;

  vkCmdBeginRenderPass(command_buffer_, &render_pass_begin_info,
                       VK_SUBPASS_CONTENTS_INLINE);
}

void CommandBuffer::setViewPort(
    const std::initializer_list<VkViewport> &viewports) {
  vkCmdSetViewport(command_buffer_, 0, viewports.size(), viewports.begin());
}

void CommandBuffer::setScissor(
    const std::initializer_list<VkRect2D> &scissors) {
  vkCmdSetScissor(command_buffer_, 0, scissors.size(), scissors.begin());
}

void CommandBuffer::bindPipeline(
    const std::shared_ptr<Pipeline> &pipeline)
{
  auto pipeline_bind_point = pipeline->getType() == Pipeline::Type::GRAPHICS
                                 ? VK_PIPELINE_BIND_POINT_GRAPHICS
                                 : VK_PIPELINE_BIND_POINT_COMPUTE;
  vkCmdBindPipeline(command_buffer_, pipeline_bind_point,
                    pipeline->getHandle());
}

void CommandBuffer::bindDescriptorSets(
    const std::shared_ptr<Pipeline> &pipeline,
    const std::initializer_list<std::shared_ptr<DescriptorSet>>
        &descriptor_sets,
    const std::initializer_list<uint32_t> &dynamic_offsets,
    const uint32_t first_set) {
  if (descriptor_sets.size() == 0) return;
  std::vector<VkDescriptorSet> ds(descriptor_sets.size());
  for (auto i = 0; i < descriptor_sets.size(); ++i) {
    ds[i] = descriptor_sets.begin()[i]->getHandle();
  }
  auto pipeline_bind_point = pipeline->getType() == Pipeline::Type::GRAPHICS
                                 ? VK_PIPELINE_BIND_POINT_GRAPHICS
                                 : VK_PIPELINE_BIND_POINT_COMPUTE;  
  vkCmdBindDescriptorSets(command_buffer_, pipeline_bind_point,
                          pipeline->getPipelineLayout()->getHandle(), first_set,
                          descriptor_sets.size(), ds.data(),
                          dynamic_offsets.size(), dynamic_offsets.begin());
}

void CommandBuffer::bindVertexBuffers(
    const std::initializer_list<std::shared_ptr<Buffer>> &buffers,
    const std::initializer_list<VkDeviceSize> &offsets,
    const uint32_t first_binding) {
  std::vector<VkBuffer> bs(buffers.size());
  for (auto i = 0; i < buffers.size(); ++i) {
    bs[i] = buffers.begin()[i]->getHandle();
  }
  vkCmdBindVertexBuffers(command_buffer_, first_binding, buffers.size(),
                         bs.data(), offsets.begin());
}

void CommandBuffer::bindIndexBuffer(const std::shared_ptr<Buffer> &buffer,
                                    const VkDeviceSize offset,
                                    const VkIndexType index_type) {
  vkCmdBindIndexBuffer(command_buffer_, buffer->getHandle(), offset,
                       index_type);
}

void CommandBuffer::draw(const uint32_t vertex_count,
                         const uint32_t instance_count,
                         const uint32_t first_vertex,
                         const uint32_t first_instance) {
  vkCmdDraw(command_buffer_, vertex_count, instance_count, first_vertex,
            first_instance);
}

void CommandBuffer::drawIndexed(const uint32_t index_count,
                                const uint32_t instance_count,
                                const uint32_t first_index,
                                const int32_t vertex_offset,
                                const uint32_t first_instance) {
  vkCmdDrawIndexed(command_buffer_, index_count, instance_count, first_index,
                   vertex_offset, first_instance);
}

void CommandBuffer::endRenderPass() { vkCmdEndRenderPass(command_buffer_); }

void CommandBuffer::imageMemoryBarrier(
    const ImageMemoryBarrier &image_memory_barrier,
    const std::shared_ptr<ImageView> &image_view) {
  VkImageMemoryBarrier barrier{
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .pNext = nullptr,
      .srcAccessMask = image_memory_barrier.src_access_mask,
      .dstAccessMask = image_memory_barrier.dst_access_mask,
      .oldLayout = image_memory_barrier.old_layout,
      .newLayout = image_memory_barrier.new_layout,
      .srcQueueFamilyIndex = image_memory_barrier.src_queue_family_index,
      .dstQueueFamilyIndex = image_memory_barrier.dst_queue_family_index,
      .image = image_view->getImage()->getHandle(),
      .subresourceRange = image_view->getSubresourceRange()};

  vkCmdPipelineBarrier(command_buffer_, image_memory_barrier.src_stage_mask,
                       image_memory_barrier.dst_stage_mask, 0, 0, nullptr, 0,
                       nullptr, 1, &barrier);
  image_view->updateLayout(image_memory_barrier.new_layout);
}

void CommandBuffer::imageMemoryBarrier(
    const ImageMemoryBarrier &image_memory_barrier,
    const std::shared_ptr<Image> &image) {
  VkImageSubresourceRange range{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                .levelCount = 1,
                                .baseArrayLayer = 0,
                                .layerCount = 1};
  VkImageMemoryBarrier barrier{
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .pNext = nullptr,
      .srcAccessMask = image_memory_barrier.src_access_mask,
      .dstAccessMask = image_memory_barrier.dst_access_mask,
      .oldLayout = image_memory_barrier.old_layout,
      .newLayout = image_memory_barrier.new_layout,
      .srcQueueFamilyIndex = image_memory_barrier.src_queue_family_index,
      .dstQueueFamilyIndex = image_memory_barrier.dst_queue_family_index,
      .image = image->getHandle(),
      .subresourceRange = range};

  vkCmdPipelineBarrier(command_buffer_, image_memory_barrier.src_stage_mask,
                       image_memory_barrier.dst_stage_mask, 0, 0, nullptr, 0,
                       nullptr, 1, &barrier);
  image->updateLayout(image_memory_barrier.new_layout);
}

void CommandBuffer::imageMemoryBarriers(
    const std::vector<ImageMemoryBarrier> &image_memory_barriers,
    const std::vector<std::shared_ptr<ImageView>> &image_views) {
  assert(image_memory_barriers.size() == image_views.size());
  std::vector<VkImageMemoryBarrier> barriers(image_memory_barriers.size());
  for (auto i = 0; i < image_memory_barriers.size(); ++i) {
    auto &image_memory_barrier = image_memory_barriers[i];
    auto image = image_views[i]->getImage();
    auto range = image_views[i]->getSubresourceRange();
    barriers[i] = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = image_memory_barrier.src_access_mask,
        .dstAccessMask = image_memory_barrier.dst_access_mask,
        .oldLayout = image_memory_barrier.old_layout,
        .newLayout = image_memory_barrier.new_layout,
        .srcQueueFamilyIndex = image_memory_barrier.src_queue_family_index,
        .dstQueueFamilyIndex = image_memory_barrier.dst_queue_family_index,
        .image = image->getHandle(),
        .subresourceRange = range};
  }
  vkCmdPipelineBarrier(command_buffer_, image_memory_barriers[0].src_stage_mask,
                       image_memory_barriers[0].dst_stage_mask, 0, 0, nullptr,
                       0, nullptr, barriers.size(), barriers.data());
}

void CommandBuffer::pushConstants(const std::shared_ptr<Pipeline> &pipeline,
                                  const VkShaderStageFlags stage_flags,
                                  const uint32_t offset, const uint32_t size,
                                  const void *data) {
  vkCmdPushConstants(command_buffer_,
                     pipeline->getPipelineLayout()->getHandle(), stage_flags,
                     offset, size, data);
}
} // namespace mango