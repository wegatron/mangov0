#pragma once

#include <Eigen/Dense>
#include <chrono>
#include <engine/utils/vk/vk_config.h>
#include <memory>
#include <vector>

namespace mango {
class EngineContext {
public:
  EngineContext() = default;
  ~EngineContext() = default;

  bool init(const std::shared_ptr<class VkConfig> &vk_config,
            const std::shared_ptr<class Window> &window);

  void destroy();

  float calcDeltaTime();
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
  const auto &getGPUAssetManager() const { return gpu_asset_manager_; }

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
  std::shared_ptr<class GPUAssetManager> gpu_asset_manager_;
  std::chrono::steady_clock::time_point last_tick_time_point_;
};

extern EngineContext g_engine;
} // namespace mango