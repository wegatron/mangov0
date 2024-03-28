#include <engine/utils/vk/vk_common.h>

namespace mango {
VulkanLayoutTransition
blitterTransitionHelper(VulkanLayoutTransition transition) {
  switch (transition.newLayout) {
  case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
  case VK_IMAGE_LAYOUT_GENERAL:
    transition.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    transition.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    transition.srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    transition.dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    break;

  case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
  case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
  default:
    transition.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    transition.dstAccessMask = 0;
    transition.srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    transition.dstStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    break;
  }
  return transition;
}

VulkanLayoutTransition
textureTransitionHelper(VulkanLayoutTransition transition) {
  switch (transition.newLayout) {
  case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
    transition.srcAccessMask = 0;
    transition.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    transition.srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    transition.dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    break;
  case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
    transition.srcAccessMask = 0;
    transition.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    transition.srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    transition.dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    break;
  case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
  case VK_IMAGE_LAYOUT_GENERAL:
  case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
    transition.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    transition.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    transition.srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    transition.dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    break;

    // We support PRESENT as a target layout to allow blitting from the swap
    // chain. See also SwapChain::makePresentable().
  case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
  case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
    transition.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    transition.dstAccessMask = 0;
    transition.srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    transition.dstStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    break;

  default:
    throw VulkanUseException("Unsupported layout transition.");
  }
  return transition;
}

void transitionImageLayout(VkCommandBuffer cmdbuffer,
                           VulkanLayoutTransition transition) {
  if (transition.oldLayout == transition.newLayout) {
    return;
  }
  assert(transition.image != VK_NULL_HANDLE && "Please call bindToSwapChain.");
  VkImageMemoryBarrier barrier = {};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = transition.oldLayout;
  barrier.newLayout = transition.newLayout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = transition.image;
  barrier.subresourceRange = transition.subresources;
  barrier.srcAccessMask = transition.srcAccessMask;
  barrier.dstAccessMask = transition.dstAccessMask;
  vkCmdPipelineBarrier(cmdbuffer, transition.srcStage, transition.dstStage, 0,
                       0, nullptr, 0, nullptr, 1, &barrier);
}
} // namespace mango