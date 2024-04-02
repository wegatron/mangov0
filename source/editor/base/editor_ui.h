#pragma once

#include <Eigen/Dense>
#include <IconsFontAwesome5.h>
#include <imgui/imgui.h>
#include <map>
#include <memory>
#include <string>
#include <volk.h>

// #include "engine/resource/asset/texture_2d.h"

namespace mango {
class Sampler;
class ImageView;
struct ImGuiImage {
  std::shared_ptr<ImageView> image_view;
  std::shared_ptr<Sampler> sampler;
  VkDescriptorSet tex_id;

  ~ImGuiImage();
};

class EditorUI {
public:
  EditorUI() = default;
  virtual ~EditorUI() = default;
  virtual void init();
  virtual void construct() = 0;
  virtual void onWindowResize() {}
  virtual void onWindowRepos() {}

protected:
  void updateWindowRegion();
  std::shared_ptr<ImGuiImage>
  loadImGuiImageFromFile(const std::string &filename);
  // std::shared_ptr<ImGuiImage>
  // loadImGuiImageFromTexture2D(std::shared_ptr<class Texture2D> &texture);
  std::shared_ptr<ImGuiImage> loadImGuiImageFromImageViewSampler(
      const std::shared_ptr<ImageView> &image_view,
      const std::shared_ptr<Sampler> &sampler);
  std::shared_ptr<ImGuiImage> getImGuiImageFromCache(const std::string &url);
  ImFont *defaultFont();
  ImFont *smallFont();
  ImFont *bigIconFont();

  bool isFocused();
  bool isPoppingUp();
  bool isImGuiImageLoaded(const std::string &url);

  std::string title_;
  char title_buf_[128];
  Eigen::Vector<int32_t, 4> content_region_;

private:
  bool isMouseFocused();

  std::shared_ptr<Sampler> texture_2d_sampler_;
  std::map<std::string, std::shared_ptr<ImGuiImage>> imgui_images_;
};
} // namespace mango