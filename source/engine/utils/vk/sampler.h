#pragma once

#include <engine/utils/vk/vk_driver.h>
#include <memory>
#include <volk.h>

namespace mango {
class Sampler {
public:
  /**
   * \brief create vulkan sampler.
   * \param mag_filter
   * \param min_filter. specify the filtering/interpolation mode to be used when
   * the image is magnified or minified. for example: VK_FILTER_NEAREST,
   * VK_FILTER_LINEAR \param mipmap_mode. Specifies the filtering/interpolation
   * method between mipmaps. for example: VK_SAMPLER_MIPMAP_MODE_NEAREST,
   * VK_SAMPLER_MIPMAP_MODE_LINEAR \param address_mode_u \param address_mode_v.
   * used to transform coordinate that is outside the image. for example:
   * VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
   * VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
   *    VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER
   */
  Sampler(const std::shared_ptr<VkDriver> &driver, VkFilter mag_filter,
          VkFilter min_filter, VkSamplerMipmapMode mipmap_mode,
          VkSamplerAddressMode address_mode_u,
          VkSamplerAddressMode address_mode_v);

  VkSampler getHandle() const noexcept { return handle_; }

  ~Sampler();

private:
  std::shared_ptr<VkDriver> driver_;
  VkSampler handle_{VK_NULL_HANDLE};
};
} // namespace mango