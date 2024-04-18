#pragma once

#include <cstdint>
#include <engine/functional/render/pass/render_data.h>
#include <memory>

namespace mango {
class CommandBuffer;
class GraphicsPipeline;
class RenderPass;
class CustomRenderPass {
public:
  CustomRenderPass() = default;
  virtual ~CustomRenderPass() = default;

  /**
   * @brief initialize render pass object.
   * create pipeline state, descriptor set etc.
   */
  virtual void init() = 0;

  virtual void render(const std::shared_ptr<CommandBuffer> &cmd_buffer) = 0;

  /**
   * @brief update render settings when swapchain object is
   * created/recreated.
   * @param width
   * @param height
   */
  virtual void onCreateSwapchainObject(const uint32_t width,
                                       const uint32_t height) {
    width_ = width;
    height_ = height;
  }

protected:
  std::shared_ptr<GraphicsPipeline> pipeline_;
  std::shared_ptr<RenderPass> render_pass_;
  uint32_t width_{0};
  uint32_t height_{0};
};
} // namespace mango