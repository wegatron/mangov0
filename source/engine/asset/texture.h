#pragma once

#include <cereal/access.hpp>
#include <engine/asset/url.h>
#include <vector>
#include <volk.h>

namespace mango {

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

class AssetTexture {
public:
  AssetTexture();
  virtual ~AssetTexture();

  void setAddressMode(VkSamplerAddressMode address_mode) {
    address_mode_u_ = address_mode;
    address_mode_v_ = address_mode;
    address_mode_w_ = address_mode;
  }

  void loadTexture(const URL &url);

protected:
  uint32_t width_, height_;
  uint32_t mip_levels_;
  uint32_t layers_;
  VkFilter min_filter_, mag_filter_;
  VkSamplerAddressMode address_mode_u_, address_mode_v_, address_mode_w_;

  ETextureCompressionMode compression_mode_;
  ETextureType texture_type_; //!< will be used for texture compression strategy
  VkFormat pixel_format_;
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
    ar(cereal::make_nvp("pixel_format", pixel_format_));
    ar(cereal::make_nvp("image_data", image_data_));
  }
};
} // namespace mango