#pragma once

#include <cstdint>
#include <memory>

namespace mango {
class CustomRenderPass {
public:
  CustomRenderPass() = default;
  virtual ~CustomRenderPass() = default;

  /**
   * @brief initialize render pass object.
   * create pipeline state, descriptor set, frame buffer, etc.
   */
  virtual void init() = 0;

  virtual void draw(const std::shared_ptr<class CommandBuffer> &cmd_buffer) = 0;

  /**
   * @brief update render settings when swapchain object is created/recreated.
   * @param width
   * @param height
   */
  virtual void onCreateSwapchainObject(const uint32_t width,
                                       const uint32_t height) = 0;

protected:
  std::shared_ptr<class GraphicsPipeline> pipeline_;
};
} // namespace mango