#pragma once

#include <engine/functional/render/pass/main_pass.h>
#include <engine/functional/render/pass/render_data.h>
#include <engine/functional/render/pass/ui_pass.h>
#include <engine/utils/vk/syncs.h>
#include <vector>
#include <list>
#include <mutex>

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

  std::shared_ptr<Semaphore> getFreeSemaphore();


  /**
   * @brief release the exec semaphores from last commit   
   */
  void releaseExecSemaphores(uint32_t frame_index)
  {
    std::lock_guard<std::mutex> lock(semaphores_mtx_);
    free_semaphores_.splice(free_semaphores_.end(), exec_semaphores_[frame_index]);
  }
  
  void addPendingSemaphore(uint32_t frame_index,
                           const std::shared_ptr<Semaphore> &semaphore)
  {
    std::lock_guard<std::mutex> lock(semaphores_mtx_);
    pending_semaphores_[frame_index].push_back(semaphore);
  }
  
  /**
   * @brief Get the Pending Semaphores and add them to exec_semaphores
   * 
   * @param frame_index   
   */
  const std::vector<VkSemaphore> getPendingSemaphores(uint32_t cur_frame_index, uint32_t prev_frame_index)
  {
    std::lock_guard<std::mutex> lock(semaphores_mtx_);
    exec_semaphores_[cur_frame_index].splice(exec_semaphores_[cur_frame_index].end(), pending_semaphores_[prev_frame_index]);
    std::vector<VkSemaphore> waiting_semaphores;
    waiting_semaphores.reserve(exec_semaphores_[cur_frame_index].size() + 1);
    for (auto &semaphore : exec_semaphores_[cur_frame_index])
      waiting_semaphores.emplace_back(semaphore->getHandle());
    return waiting_semaphores;
  }
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
  std::list<std::shared_ptr<Semaphore>> free_semaphores_;
  std::list<std::shared_ptr<Semaphore>> pending_semaphores_[MAX_FRAMES_IN_FLIGHT];
  std::list<std::shared_ptr<Semaphore>> exec_semaphores_[MAX_FRAMES_IN_FLIGHT];
};
} // namespace mango