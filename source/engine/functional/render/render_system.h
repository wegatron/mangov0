#pragma once

#include <engine/functional/render/pass/ui_pass.h>
#include <engine/utils/vk/syncs.h>
#include <vector>

namespace mango {
class ImageView;
class RenderSystem {
public:
  RenderSystem() = default;
  ~RenderSystem() = default;

  /**
   * @brief init render system: create descriptor pool, global param set,
   * resource cache, stage pool, render pass, frame buffer, etc.
   */
  void init();

  void tick(float delta_time);

  std::shared_ptr<ImageView> getColorImageView() const {
    return color_image_view_;
  }

  /**
   * @brief recreate color and depth image view after resize
   *
   * @param width
   * @param height
   */
  void resize3DView(int width, int height);

  // void beginFrame(const float time_elapse, const uint32_t frame_index,
  //                 const uint32_t rt_index);

  // void render(World *scene, Gui *gui);

  // void endFrame();

private:
  /**
   * @brief update frame buffer's color attachment after swapchain image
   * recreated
   */
  void onCreateSwapchainObjects(const std::shared_ptr<class Event> &event);

  std::unique_ptr<UIPass> ui_pass_;

  std::shared_ptr<ImageView> color_image_view_;
  std::shared_ptr<ImageView> depth_image_view_;
  // uint32_t cur_frame_index_{0};
  // uint32_t cur_rt_index_{0};
  // float cur_time_{0.0};
  // std::shared_ptr<CommandBuffer> cmd_buf_;
  // RPass rpass_;
  // std::vector<std::unique_ptr<class FrameBuffer>> frame_buffers_;
};
} // namespace mango