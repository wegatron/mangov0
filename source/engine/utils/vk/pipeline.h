#pragma once

#include <cassert>
#include <engine/utils/vk/pipeline_state.h>

namespace mango {
class ResourceCache;
class RenderPass;
class Pipeline {
public:
  enum class Type {
    UNKNOWN = 0,
    GRAPHICS,
    COMPUTE,
    // RAY_TRACYING
  };
  Pipeline(const Pipeline &) = delete;
  Pipeline &operator=(const Pipeline &) = delete;

  Pipeline(const std::shared_ptr<VkDriver> &driver, const Type type)
      : driver_(driver), type_(type) {}

  virtual ~Pipeline() = default;

  VkPipeline getHandle() const { return pipeline_; }

  std::shared_ptr<PipelineLayout> getPipelineLayout() const {
    return pipeline_layout_;
  }

  Type getType() const { return type_; }

protected:
  Type type_;
  VkPipeline pipeline_{VK_NULL_HANDLE};
  std::shared_ptr<VkDriver> driver_;
  std::shared_ptr<PipelineLayout> pipeline_layout_;
};

class GraphicsPipeline : public Pipeline {
public:
  GraphicsPipeline(const std::shared_ptr<VkDriver> &driver,
                   const std::shared_ptr<ResourceCache> &cache,
                   const std::shared_ptr<RenderPass> &render_pass,
                   std::unique_ptr<GPipelineState> &&pipeline_state);

  GPipelineState &getPipelineState() const {
    assert(pipeline_state_ != nullptr);
    return *pipeline_state_;
  }

  ~GraphicsPipeline() override;

  void cleanDirtyFlag() { pipeline_state_->dirty_ = false; }

private:
  std::unique_ptr<GPipelineState> pipeline_state_;
};
} // namespace mango