#pragma once

#include <engine/functional/render/pass/render_data.h>
#include <engine/functional/render/pass/render_pass.h>
namespace mango {
class RenderData;
class FrameBuffer;
class MainPass final : public CustomRenderPass {
public:
  MainPass() = default;
  ~MainPass() override = default;

  void init() override;

  void render(const std::shared_ptr<CommandBuffer> &cmd_buffer) override;

  void setRenderData(const std::shared_ptr<RenderData> &render_data) {
    render_data_ = render_data;
  }

  void setFrameBuffer(const std::shared_ptr<FrameBuffer> &frame_buffer,
                      const int width, const int height) {
    frame_buffer_ = frame_buffer;
    width_ = width;
    height_ = height;
  }

protected:
  void draw(const std::shared_ptr<CommandBuffer> &cmd_buffer,
            const std::vector<StaticMeshRenderData> &static_meshs);
  std::shared_ptr<RenderData> render_data_;
  std::shared_ptr<FrameBuffer> frame_buffer_;
};
} // namespace mango