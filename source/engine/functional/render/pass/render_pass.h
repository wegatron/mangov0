#pragma once

#include <cstdint>
#include <memory>

namespace mango {
class CustomRenderPass {
public:
  CustomRenderPass() = default;
  virtual ~CustomRenderPass() = default;

  virtual void init() = 0;

  virtual void draw(const std::shared_ptr<class CommandBuffer> &cmd_buffer) = 0;

  /**
   * @brief update render settings when swapchain object is created/recreated.
   * @param width
   * @param height
   */
  virtual void onCreateSwapchainObject(const uint32_t width,
                                       const uint32_t height) = 0;
};
} // namespace mango