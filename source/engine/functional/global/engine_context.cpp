#include <engine/asset/asset_manager.h>
#include <engine/functional/global/engine_context.h>
#include <engine/functional/render/render_system.h>
#include <engine/functional/world/world.h>
#include <engine/functional/global/resource_binding_mgr.h>
#include <engine/platform/file_system.h>
#include <engine/platform/glfw_window.h>
#include <engine/utils/base/timer.h>
#include <engine/utils/event/event_system.h>
#include <engine/utils/log/log_system.h>
#include <engine/utils/vk/resource_cache.h>
#include <engine/utils/vk/stage_pool.h>
#include <engine/utils/vk/vk_driver.h>
#include <engine/utils/base/macro.h>


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
  timer_manager_ = std::make_shared<TimerManager>();
  timer_manager_->init();

  // window
  window_ = std::make_shared<GlfwWindow>(window_title, 800, 600);

  // vulkan driver
  driver_ = std::make_shared<VkDriver>();
  driver_->init();

  // resource cache
  resource_cache_ = std::make_shared<ResourceCache>();
  resource_cache_->init(driver_);

  // asset manager
  asset_manager_ = std::make_shared<AssetManager>();
  asset_manager_->init();

  // resource binding manager
  resource_binding_mgr_ = std::make_shared<ResourceBindingMgr>(driver_);

  // render system
  render_system_ = std::make_shared<RenderSystem>();
  render_system_->init();

  // world manager
  world_ = std::make_shared<World>();

  driver_->initThreadLocalCommandBufferManagers(
      { driver_->getGraphicsQueue()->getFamilyIndex(), driver_->getTransferQueue()->getFamilyIndex() });
  
  driver_->setThreadLocalCommandBufferManagerTid(0, std::this_thread::get_id());
  // animation & physics manager
  event_process_thread_ = new std::thread([this]() {
    driver_->setThreadLocalCommandBufferManagerTid(1, std::this_thread::get_id());
    auto &cmd_buffer_mgr = driver_->getThreadLocalCommandBufferManager();
    // wait cv from main thread and do tick once
    while (!is_exit_) {
      sem_event_process_start_.acquire();
      if(is_exit_) break;
      cmd_buffer_mgr.getCommandBufferAvailableFence()->wait();
      event_system_->tick();
      // commit command buffer if have
      if(cmd_buffer_mgr.needCommit())
      {
        auto render_system = g_engine.getRenderSystem();
        auto semaphore = render_system->getFreeSemaphore();
        cmd_buffer_mgr.commitExecutableCommandBuffers(driver_->getTransferQueue(), semaphore);
        render_system->addPendingSemaphore(driver_->getCurFrameIndex(), semaphore);
      }
      sem_event_process_finish_.release();
    }
  });
  return true;
}

void EngineContext::destroy() {
  is_exit_ = true;
  if(event_process_thread_ != nullptr)
  {
    sem_event_process_start_.release();
    event_process_thread_->join();
  }  
  resource_cache_.reset();
  render_system_.reset();
  world_.reset();
  resource_binding_mgr_.reset();
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
void EngineContext::gcTick(float delta_time) { 
  driver_->getStagePool()->gc();
  resource_cache_->gc();
}

void EngineContext::logicTick(float delta_time) {
  world_->tick(delta_time);
}
void EngineContext::renderTick(float delta_time) {
  render_system_->tick(delta_time);
}

} // namespace mango