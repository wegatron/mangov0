#pragma once

#include <memory>
#include <string>
#include <vector>

#include <engine/functional/global/app_context.h>
#include <engine/platform/input_events.h>
#include <engine/utils/vk/frame_buffer.h>
#include <engine/utils/vk/resource_cache.h>
#include <engine/utils/vk/syncs.h>

namespace mango {

class VkDriver;
class Fence;
class Semaphore;

class AppBase {
public:
  AppBase(const std::string &name) : name_(name) {}

  virtual ~AppBase() = default;

  AppBase(const AppBase &) = delete;
  AppBase &operator=(const AppBase &) = delete;

  virtual void tick(const float seconds, const uint32_t rt_index,
                    const uint32_t frame_index) = 0;

  /**
   * \brief init graphics rendering stuff and setup framebuffer.
   */
  virtual void init(Window *window, const std::shared_ptr<VkDriver> &driver,
                    const std::vector<std::shared_ptr<RenderTarget>> &rts) = 0;

  /**
   * \brief update the render target in frame buffer, also with camera aspect
   * ratio.
   */
  virtual void
  updateRts(const std::vector<std::shared_ptr<RenderTarget>> &rts) = 0;

  /**
   * \brief using the event manager to process event.
   */
  virtual void
  inputMouseEvent(const std::shared_ptr<MouseInputEvent> &mouse_event) {}

protected:
  std::string name_;
};

} // namespace mango