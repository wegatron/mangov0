#pragma once

#include <engine/utils/base/error.h>
#include <volk.h>

namespace mango {
//! returns whether this format a depth format
static constexpr bool isDepthFormat(VkFormat format) noexcept {
  switch (format) {
  case VK_FORMAT_D16_UNORM_S8_UINT:
  case VK_FORMAT_D32_SFLOAT:
  case VK_FORMAT_D16_UNORM:
  case VK_FORMAT_D32_SFLOAT_S8_UINT:
  case VK_FORMAT_D24_UNORM_S8_UINT:
    return true;
  default:
    return false;
  }
}

struct VulkanLayoutTransition {
  VkImage image;
  VkImageLayout oldLayout;
  VkImageLayout newLayout;
  VkImageSubresourceRange subresources;
  VkPipelineStageFlags srcStage;
  VkAccessFlags srcAccessMask;
  VkPipelineStageFlags dstStage;
  VkAccessFlags dstAccessMask;
};

VulkanLayoutTransition
blitterTransitionHelper(VulkanLayoutTransition transition);

VulkanLayoutTransition
textureTransitionHelper(VulkanLayoutTransition transition);

void transitionImageLayout(VkCommandBuffer cmdbuffer,
                           VulkanLayoutTransition transition);

} // namespace mango