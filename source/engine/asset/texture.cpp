#include <engine/asset/texture.h>
#include <stb_image.h>

namespace mango {

void AssetTexture::loadTexture(const URL &url) {
  std::string extension = url.getExtension();
  std::string absolute_path = url.getAbsolute();
  if (extension == "ktx") {
    // load ktx texture
  } else if (extension == "png") {
    int channel = 0;
    stbi_uc *img_data =
        stbi_load(absolute_path.c_str(), reinterpret_cast<int32_t *>(&width_),
                  reinterpret_cast<int32_t *>(&height_),
                  reinterpret_cast<int *>(&channel), 0);
    auto ret = createImageView(img_data, width_, height_, channel, cmd_buf);
    stbi_image_free(img_data);
  } else {
    throw std::runtime_error("unsupported texture format");
  }
}
} // namespace mango