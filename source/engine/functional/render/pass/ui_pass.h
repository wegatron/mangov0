#pragma once
#include <engine/functional/render/pass/render_pass.h>
#include <engine/utils/vk/framebuffer.h>

namespace mango {

/**
 * @brief 将最终渲染结果和UI内容一起绘制到屏幕上
 */
class UIPass final : public CustomRenderPass {
public:
  UIPass() = default;
  ~UIPass();

  /**
   * @brief create render stuffs including Graphics pipeline, RenderPass,
   * FrameBuffer, DescriptorSet, buffers/images.
   */
  void init() override;
  void prepare();
  void render(const std::shared_ptr<CommandBuffer> &cmd_buffer) override;

  void onCreateSwapchainObject(const uint32_t width,
                               const uint32_t height) override;

private:
  void initImgui();
  void createRenderPassAndFramebuffer();

  std::shared_ptr<class RenderPass> render_pass_;
};
} // namespace mango