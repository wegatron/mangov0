#pragma once

#include <cstdint>
#include <engine/functional/render/pass/render_data.h>
#include <memory>

namespace mango {
class CommandBuffer;
class CustomRenderPass {
public:
  CustomRenderPass() = default;
  virtual ~CustomRenderPass() = default;

  /**
   * @brief initialize render pass object.
   * create pipeline state, descriptor set etc.
   */
  virtual void init() = 0;

  void render(const std::shared_ptr<CommandBuffer> &cmd_buffer,
              const RenderData &render_data) {
    render(cmd_buffer, render_data.static_mesh_render_data);
  }

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
  virtual void render(const std::shared_ptr<CommandBuffer> &cmd_buffer,
                      const std::vector<StaticMesh> &static_meshs) = 0;

  std::shared_ptr<class GraphicsPipeline> pipeline_;
  uint32_t width_{0};
  uint32_t height_{0};
};
} // namespace mango