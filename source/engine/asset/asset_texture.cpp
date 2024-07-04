#include <engine/asset/asset_texture.h>
#include <engine/functional/global/engine_context.h>
#include <engine/utils/base/macro.h>
#include <engine/utils/vk/commands.h>
#include <engine/utils/vk/data_uploader.hpp>
#include <engine/utils/vk/image.h>
#include <stb_image.h>

namespace mango {

void AssetTexture::load(const URL &url) {
  url_ = url;
  std::string extension = url.getExtension();
  std::string absolute_path = url.getAbsolute();
  if (extension == "ktx") {
    // load ktx texture
  } else if (extension == "png" || extension == "jpg" || extension == "jpeg") {
    stbi_uc *img_data =
        stbi_load(absolute_path.c_str(), reinterpret_cast<int *>(&width_),
                  reinterpret_cast<int *>(&height_), 0, 4);
    if (img_data == nullptr) {
      throw std::runtime_error("failed to load texture");
    }
    layers_ = mip_levels_ = 1;
    image_data_.resize(width_ * height_ * 4);
    memcpy(image_data_.data(), img_data, image_data_.size());
    stbi_image_free(img_data);
  } else {
    throw std::runtime_error("unsupported texture file format");
  }
  inflate();
}

void AssetTexture::load(uint32_t width, uint32_t height, stbi_uc *data) {
  if (data == nullptr) {
    throw std::runtime_error("failed to load texture");
  }
  layers_ = mip_levels_ = 1;
  width_  = width; height_ = height;
  image_data_.resize(width_ * height_ * 4);
  memcpy(image_data_.data(), data, image_data_.size());
  inflate();
}

void AssetTexture::inflate() {
  auto pixel_format = getFormat();
  if (compression_mode_ == ETextureCompressionMode::None) {
    auto &cmd_buffer_mgr =
        g_engine.getDriver()->getThreadLocalCommandBufferManager();
    auto cmd_buffer = cmd_buffer_mgr.requestCommandBuffer(
        VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    image_view_ = uploadImage(image_data_.data(), width_, height_, mip_levels_,
                              layers_, pixel_format, cmd_buffer);
  } else {
    // compress image data to GPU
  }
}

bool AssetTexture::isSRGB() {
  switch (texture_type_) {
  case ETextureType::BaseColor:
  case ETextureType::Emissive:
    return true;
  default:
    return false;
  }
}

VkFormat AssetTexture::getFormat() {
  bool is_srgb = isSRGB();
  switch (pixel_type_) {
  case EPixelType::RGBA8:
    return is_srgb ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;
  case EPixelType::RGBA16:
    return VK_FORMAT_R16G16B16A16_SFLOAT;
  case EPixelType::RGBA32:
    return VK_FORMAT_R32G32B32A32_SFLOAT;
  case EPixelType::R16:
    return VK_FORMAT_R16_SFLOAT;
  case EPixelType::R32:
    return VK_FORMAT_R32_SFLOAT;
  case EPixelType::RG16:
    return VK_FORMAT_R16G16_SFLOAT;
  default:
    throw std::runtime_error("unsupported pixel type");
    return VK_FORMAT_R8G8B8A8_SRGB;
  }
}
} // namespace mango