#pragma once

#include <Eigen/Dense>
#include <chrono>
#include <engine/utils/vk/vk_config.h>
#include <memory>
#include <vector>
#include <thread>
#include <semaphore>

namespace mango {
class EngineContext {
public:
  EngineContext() = default;
  ~EngineContext() = default;

  bool init(const std::shared_ptr<class VkConfig> &vk_config,
            const std::string &window_title);

  void destroy();

  /**
   * @brief sync of other threads
   */
  void threadSync()
  {
    sem_event_process_finish_.acquire();
  }

  void newTick() { sem_event_process_start_.release(); }

  float calcDeltaTime();
  void gcTick(float delta_time);
  void logicTick(float delta_time);
  void renderTick(float delta_time);

  const auto &getVkConfig() const { return vk_config_; }
  const auto &getWindow() const { return window_; }
  const auto &getDriver() const { return driver_; }
  const auto &getFileSystem() const { return file_system_; }
  const auto &getEventSystem() const { return event_system_; }
  const auto &getLogSystem() const { return log_system_; }
  const auto &getResourceCache() const { return resource_cache_; }
  const auto &getTimerManager() const { return timer_manager_; }
  const auto &getAssetManager() const { return asset_manager_; }
  const auto &getWorld() const { return world_; }
  const auto &getRenderSystem() const { return render_system_; }

private:
  std::shared_ptr<class VkConfig> vk_config_;
  std::shared_ptr<class Window> window_;
  std::shared_ptr<class VkDriver> driver_;
  std::shared_ptr<class ResourceCache> resource_cache_;
  std::shared_ptr<class FileSystem> file_system_;
  std::shared_ptr<class EventSystem> event_system_;
  std::shared_ptr<class LogSystem> log_system_;
  std::shared_ptr<class RenderSystem> render_system_;
  std::shared_ptr<class TimerManager> timer_manager_;
  std::shared_ptr<class AssetManager> asset_manager_;
  std::shared_ptr<class World> world_;
  std::chrono::steady_clock::time_point last_tick_time_point_;
  std::thread *event_process_thread_ {nullptr};

  std::binary_semaphore sem_event_process_finish_{0};
  std::binary_semaphore sem_event_process_start_{0};
  bool is_exit_{false};
};

extern EngineContext g_engine;
} // namespace mango