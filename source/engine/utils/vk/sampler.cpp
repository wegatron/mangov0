#include <engine/utils/base/error.h>
#include <engine/utils/vk/sampler.h>

namespace mango {
Sampler::Sampler(const std::shared_ptr<VkDriver> &driver, VkFilter mag_filter,
                 VkFilter min_filter, VkSamplerMipmapMode mipmap_mode,
                 VkSamplerAddressMode address_mode_u,
                 VkSamplerAddressMode address_mode_v)
    : driver_(driver) {
  VkSamplerCreateInfo info{
      .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
      .pNext = NULL,
      .flags = 0,
      .magFilter = mag_filter,
      .minFilter = min_filter,
      .mipmapMode = mipmap_mode,
      .addressModeU = address_mode_u,
      .addressModeV = address_mode_v,
      .addressModeW = address_mode_v,
      .mipLodBias = 0.0f,
      .anisotropyEnable = VK_FALSE,
      .maxAnisotropy = 1.0f,
      .compareEnable = VK_FALSE,
      .compareOp = VK_COMPARE_OP_ALWAYS,
      .minLod = 0.0f,
      .maxLod = 0.0f,
      .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
      .unnormalizedCoordinates = VK_FALSE,
  };
  auto result = vkCreateSampler(driver->getDevice(), &info, nullptr, &handle_);
  VK_THROW_IF_ERROR(result, "failed to create sampler.");
}

Sampler::~Sampler() {
  if (handle_ != VK_NULL_HANDLE) {
    vkDestroySampler(driver_->getDevice(), handle_, nullptr);
  }
}
} // namespace mango