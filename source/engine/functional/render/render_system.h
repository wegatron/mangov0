#pragma once

#include <engine/functional/render/pass/main_pass.h>
#include <engine/functional/render/pass/render_data.h>
#include <engine/functional/render/pass/ui_pass.h>
#include <engine/utils/vk/syncs.h>
#include <vector>
#include <list>

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

  std::shared_ptr<ImageView> getColorImageView() const;

  /**
   * @brief recreate color and depth image view after resize
   *
   * @param width
   * @param height
   */
  void resize3DView(int width, int height);  

  std::shared_ptr<Semaphore> getFreeSemaphore(uint32_t frame_index);
  
  void releaseSemaphores(uint32_t frame_index, std::list<std::shared_ptr<Semaphore>> &semaphores);
  
  void addPendingSemaphore(uint32_t frame_index,
                           const std::shared_ptr<Semaphore> &semaphore);
  
  void getPendingSemaphores(uint32_t frame_index, std::list<std::shared_ptr<Semaphore>> &semaphore_list);
private:
  /**
   * @brief update frame buffer's color attachment after swapchain image
   * recreated
   */
  void onCreateSwapchainObjects(const std::shared_ptr<class Event> &event);

  void collectRenderDatas();

  std::unique_ptr<UIPass> ui_pass_;
  std::unique_ptr<MainPass> main_pass_;
  std::shared_ptr<FrameBuffer> frame_buffer_; //!< 3d view's frame buffer

  std::mutex semaphores_mtx_;
  std::list<std::shared_ptr<Semaphore>> free_semaphores_[MAX_FRAMES_IN_FLIGHT];
  std::list<std::shared_ptr<Semaphore>> pending_semaphores_[MAX_FRAMES_IN_FLIGHT];
  std::list<std::shared_ptr<Semaphore>> exec_semaphores_[MAX_FRAMES_IN_FLIGHT];
};
} // namespace mango