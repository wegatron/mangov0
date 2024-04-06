#include <engine/asset/asset_manager.h>
#include <engine/functional/global/engine_context.h>
#include <engine/functional/render/render_system.h>
#include <engine/platform/file_system.h>
#include <engine/platform/glfw_window.h>
#include <engine/utils/base/timer.h>
#include <engine/utils/event/event_system.h>
#include <engine/utils/log/log_system.h>
#include <engine/utils/vk/resource_cache.hpp>
#include <engine/utils/vk/vk_driver.h>

namespace mango {
EngineContext g_engine;

bool EngineContext::init(const std::shared_ptr<class VkConfig> &vk_config,
                         const std::string &window_title) {
  vk_config_ = vk_config;

  // file system
  file_system_ = std::make_shared<FileSystem>();
  file_system_->init();

  // log system
  log_system_ = std::make_shared<LogSystem>();
  log_system_->init();

  // event system
  event_system_ = std::make_shared<EventSystem>();
  event_system_->init();

  // timer manager
  g_engine.timer_manager_ = std::make_shared<TimerManager>();
  g_engine.timer_manager_->init();

  // window
  window_ = std::make_shared<GlfwWindow>(window_title, 800, 600);

  // vulkan driver
  driver_ = std::make_shared<VkDriver>();
  driver_->init();

  // resource cache
  resource_cache_ = std::make_shared<ResourceCache>();

  // asset manager
  asset_manager_ = std::make_shared<AssetManager>();
  asset_manager_->init();

  // asset manager

  // render system
  g_engine.render_system_ = std::make_shared<RenderSystem>();
  g_engine.render_system_->init();

  // world manager

  // animation & physics manager

  return true;
}

void EngineContext::destroy() {
  resource_cache_.reset();
  render_system_.reset();
  driver_->destroy();
  window_.reset();
  event_system_.reset();
  file_system_.reset();
  log_system_.reset();
  // gpu_asset_manager.reset();
  // global_param_set.reset();
}

float EngineContext::calcDeltaTime() {
  float delta_time;
  std::chrono::steady_clock::time_point tick_time_point =
      std::chrono::steady_clock::now();
  std::chrono::duration<float> time_span =
      std::chrono::duration_cast<std::chrono::duration<float>>(
          tick_time_point - last_tick_time_point_);
  delta_time = time_span.count();
  last_tick_time_point_ = tick_time_point;

  return delta_time;
}
void EngineContext::logicTick(float delta_time) {}
void EngineContext::renderTick(float delta_time) {
  render_system_->tick(delta_time);
}

} // namespace mango