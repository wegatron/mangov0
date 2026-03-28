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
#include <cstdio>
#include <cstring>


namespace mango {
EngineContext g_engine;

// Parse [GlfwWindow][Data] Width/Height from imgui.ini before GLFW window creation.
// The ini file is written back by UIPass's registered settings handler on shutdown.
static void parse_window_size_from_ini(int &width, int &height) {
  width = 1280;
  height = 720;
  FILE *f = fopen("imgui.ini", "r");
  if (!f) return;
  char line[256];
  bool in_section = false;
  while (fgets(line, sizeof(line), f)) {
    if (strncmp(line, "[GlfwWindow][Data]", 18) == 0) {
      in_section = true;
    } else if (line[0] == '[') {
      in_section = false;
    } else if (in_section) {
      int v;
      if (sscanf(line, "Width=%d", &v) == 1)       width  = v;
      else if (sscanf(line, "Height=%d", &v) == 1) height = v;
    }
  }
  fclose(f);
}

bool EngineContext::init(const std::shared_ptr<class VkConfig> &vk_config,
                         const std::string &window_title) {
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

  // window — read preferred size from imgui.ini [GlfwWindow][Data]
  int win_width, win_height;
  parse_window_size_from_ini(win_width, win_height);
  window_ = std::make_shared<GlfwWindow>(window_title, win_width, win_height);

  // vulkan driver
  driver_ = std::make_shared<VkDriver>();
  driver_->init(vk_config);

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

#ifdef IMGUI_ENABLE_TEST_ENGINE
void* EngineContext::getTestEngine() const {
  return render_system_ ? render_system_->getTestEngine() : nullptr;
}
#endif

} // namespace mango