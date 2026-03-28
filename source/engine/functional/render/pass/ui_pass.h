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

  void init() override;
  void prepare();
  void render(const std::shared_ptr<CommandBuffer> &cmd_buffer) override;
  void onCreateSwapchainObject(const uint32_t width,
                               const uint32_t height) override;

#ifdef IMGUI_ENABLE_TEST_ENGINE
  void* getTestEngine() const { return test_engine_; }
#endif

private:
  void initImgui();
  void createRenderPassAndFramebuffer();
  void createDescriptorPool();

#ifdef IMGUI_ENABLE_TEST_ENGINE
  void initTestEngine();
  void shutdownTestEngine();
  void* test_engine_ = nullptr;
#endif

  std::shared_ptr<DescriptorPool> desc_pool_;
  std::shared_ptr<class RenderPass> render_pass_;
};
} // namespace mango