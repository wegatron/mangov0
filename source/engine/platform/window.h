#pragma once

#include <string>
#include <volk.h>

namespace mango {

class Window {
public:
  Window(const std::string &window_title) : window_title_(window_title) {}

  virtual ~Window() = default;

  Window(const Window &) = delete;
  Window &operator=(const Window &) = delete;

  virtual bool shouldClose() = 0;

  /**
   * \brief event process one loop, generate user input event to callback.
   */
  virtual void processEvents() = 0;

  // virtual void initImgui() = 0;

  // virtual void shutdownImgui() = 0;

  // virtual void imguiNewFrame() = 0;

  virtual void getWindowSize(uint32_t &width, uint32_t &height) = 0;

  virtual VkSurfaceKHR createSurface(VkInstance instance) = 0;

  virtual void *getHandle() = 0;

  // virtual std::vector<std::shared_ptr<RenderTarget>> createRenderTargets() =
  // 0;

protected:
  std::string window_title_;
};
} // namespace mango