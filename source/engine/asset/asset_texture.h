#pragma once

#include <cereal/access.hpp>
#include <engine/asset/asset.h>
#include <vector>
#include <volk.h>

namespace mango {
class ImageView;
enum class ETextureCompressionMode { None, ETC1S, ASTC, ZSTD };
enum class ETextureType {
  BaseColor,
  MetallicRoughnessOcclusion,
  Normal,
  Emissive,
  Cube,
  UI,
  Data
};
enum class EPixelType { RGBA8, RGBA16, RGBA32, RG16, R16, R32 };

class AssetTexture final : public Asset {
public:
  AssetTexture() = default;
  ~AssetTexture() = default;

  void setAddressMode(VkSamplerAddressMode address_mode) {
    address_mode_u_ = address_mode;
    address_mode_v_ = address_mode;
    address_mode_w_ = address_mode;
  }

  void load(const URL &url) override;

  void setTextureType(ETextureType texture_type) {
    texture_type_ = texture_type;
  }

  std::shared_ptr<ImageView> getImageView() { return image_view_; }

  void inflate() override;
  
private:

  bool isSRGB();

  VkFormat getFormat();

  uint32_t width_{0}, height_{0};
  uint32_t mip_levels_{0};
  uint32_t layers_{0};
  VkFilter min_filter_{VK_FILTER_LINEAR}, mag_filter_{VK_FILTER_LINEAR};
  VkSamplerAddressMode address_mode_u_{VK_SAMPLER_ADDRESS_MODE_REPEAT},
      address_mode_v_{VK_SAMPLER_ADDRESS_MODE_REPEAT},
      address_mode_w_{VK_SAMPLER_ADDRESS_MODE_REPEAT};
  ETextureCompressionMode compression_mode_{ETextureCompressionMode::None};
  ETextureType texture_type_{
      ETextureType::BaseColor}; //!< will be used for texture compression
                                //!< strategy and for pixel format
  EPixelType pixel_type_{EPixelType::RGBA8};

  std::vector<uint8_t> image_data_;

  std::shared_ptr<class ImageView> image_view_;

  void uploadKtxTexture(void *p_ktx_texture,
                        VkFormat format = VK_FORMAT_UNDEFINED);

private:
  friend class cereal::access;
  template <class Archive> void serialize(Archive &ar) {
    ar(cereal::make_nvp("width", width_));
    ar(cereal::make_nvp("height", height_));
    ar(cereal::make_nvp("min_filter", min_filter_));
    ar(cereal::make_nvp("mag_filter", mag_filter_));
    ar(cereal::make_nvp("address_mode_u", address_mode_u_));
    ar(cereal::make_nvp("address_mode_v", address_mode_v_));
    ar(cereal::make_nvp("address_mode_w", address_mode_w_));
    ar(cereal::make_nvp("compression_mode", compression_mode_));
    ar(cereal::make_nvp("texture_type", texture_type_));
    ar(cereal::make_nvp("pixel_type_", pixel_type_));
    ar(cereal::make_nvp("image_data", image_data_));
  }
};
} // namespace mango