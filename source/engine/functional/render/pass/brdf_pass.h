#pragma once

#include <engine/functional/render/pass/render_pass.h>

namespace mango {
class BRDFPass final : public CustomRenderPass {
public:
  BRDFPass() = default;
  ~BRDFPass() override = default;

  void init() override;

protected:
  void render(const std::shared_ptr<CommandBuffer> &cmd_buffer,
              const std::vector<StaticMesh> &static_meshs) override;
};
} // namespace mango