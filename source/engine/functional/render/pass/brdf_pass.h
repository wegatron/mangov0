#pragma once

#include <engine/functional/render/pass/render_pass.h>

namespace mango {
class BRDFPass final : public CustomRenderPass {
public:
  BRDFPass() = default;
  ~BRDFPass() override = default;

  void init() override;

  void draw(const std::shared_ptr<class CommandBuffer> &cmd_buffer) override;

  void onCreateSwapchainObject(const uint32_t width,
                               const uint32_t height) override;
};
} // namespace mango