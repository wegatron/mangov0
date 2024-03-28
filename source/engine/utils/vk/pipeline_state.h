#pragma once

#include <engine/utils/vk/pipeline_layout.h>
#include <vector>
#include <volk.h>

namespace mango {
struct VertexInputState {
  std::vector<VkVertexInputBindingDescription> bindings;
  std::vector<VkVertexInputAttributeDescription> attributes;
  void getCreateInfo(VkPipelineVertexInputStateCreateInfo &create_info) const;
};

struct InputAssemblyState {
  VkPrimitiveTopology topology{VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};
  VkBool32 primitive_restart_enable{false};
  void getCreateInfo(VkPipelineInputAssemblyStateCreateInfo &create_info) const;
};

struct RasterizationState {
  VkBool32 depth_clamp_enable{VK_FALSE};
  VkBool32 rasterizer_discard_enable{VK_FALSE};
  VkPolygonMode polygon_mode{VK_POLYGON_MODE_FILL};
  VkCullModeFlags cull_mode{VK_CULL_MODE_BACK_BIT};
  VkFrontFace front_face{VK_FRONT_FACE_CLOCKWISE};
  VkBool32 depth_bias_enable{VK_FALSE};
  void getCreateInfo(VkPipelineRasterizationStateCreateInfo &create_info) const;
};

struct ViewPortState {
  uint32_t viewport_count{1};
  uint32_t scissor_count{1};
};

struct MultisampleState {
  VkSampleCountFlagBits rasterization_samples{VK_SAMPLE_COUNT_1_BIT};
  VkBool32 sample_shading_enable{VK_FALSE};
  float min_sample_shading{0.0f};
  VkSampleMask sample_mask{0xFFFFFFFF};
  VkBool32 alpha_to_coverage_enable{VK_FALSE};
  VkBool32 alpha_to_one_enable{VK_FALSE};

  void getCreateInfo(VkPipelineMultisampleStateCreateInfo &create_info) const;
};

struct DepthStencilState {
  VkBool32 depth_test_enable{VK_TRUE};
  VkBool32 depth_write_enable{VK_TRUE};
  VkCompareOp depth_compare_op{VK_COMPARE_OP_LESS};
  VkBool32 depth_bounds_test_enable{VK_FALSE};
  VkBool32 stencil_test_enable{VK_FALSE};
  VkStencilOpState front{};
  VkStencilOpState back{};
  float min_depth_bounds{0.0f};
  float max_depth_bounds{1.0f};

  void getCreateInfo(VkPipelineDepthStencilStateCreateInfo &create_info) const;
};

struct ColorBlendState {
  VkBool32 logic_op_enable{VK_FALSE};
  VkLogicOp logic_op{VK_LOGIC_OP_COPY};
  std::vector<VkPipelineColorBlendAttachmentState> attachments;
  float blend_constants[4]{0.0f, 0.0f, 0.0f, 0.0f};

  void getCreateInfo(VkPipelineColorBlendStateCreateInfo &create_info) const;
};

// dynamic pipeline state vulkan 1.0: viewport, scissor, line width, depth bias,
// blend constants... 1.3: depth test enable, depth write enable, depth compare
// op, depth bounds test enable...
class GPipelineState final {
public:
  GPipelineState() = default;

  GPipelineState(GPipelineState &&) = default;

  ~GPipelineState() = default;

  void
  setShaders(const std::vector<std::shared_ptr<ShaderModule>> &shader_modules);

  void setVertexInputState(const VertexInputState &state);

  void setInputAssemblyState(const InputAssemblyState &state);

  void setRasterizationState(const RasterizationState &state);

  void setViewportState(const ViewPortState &state);

  void setMultisampleState(const MultisampleState &state);

  void setDepthStencilState(const DepthStencilState &state);

  void setColorBlendState(const ColorBlendState &state);

  void setSubpassIndex(uint32_t subpass_index);

  const std::vector<std::shared_ptr<ShaderModule>> &getShaderModules() const {
    return shader_modules_;
  }

  const VertexInputState &getVertexInputState() const {
    return vertex_input_state_;
  }

  const InputAssemblyState &getInputAssemblyState() const {
    return input_assembly_state_;
  }

  const RasterizationState &getRasterizationState() const {
    return rasterization_state_;
  }

  const ViewPortState &getViewportState() const { return viewport_state_; }

  const MultisampleState &getMultisampleState() const {
    return multisample_state_;
  }

  const DepthStencilState &getDepthStencilState() const {
    return depth_stencil_state_;
  }

  const ColorBlendState &getColorBlendState() const {
    return color_blend_state_;
  }

  const uint32_t getSubpassIndex() const { return subpass_index_; }

  void getDynamicStateCreateInfo(VkPipelineDynamicStateCreateInfo &) const;

  bool isDirty() const { return dirty_; }

private:
  VertexInputState vertex_input_state_;

  InputAssemblyState input_assembly_state_;

  RasterizationState rasterization_state_;

  ViewPortState viewport_state_;

  MultisampleState multisample_state_;

  DepthStencilState depth_stencil_state_;

  ColorBlendState color_blend_state_;

  uint32_t subpass_index_{0};

  std::vector<std::shared_ptr<ShaderModule>> shader_modules_;

  std::vector<VkDynamicState> dynamic_states_{
      VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR,
      // VK_DYNAMIC_STATE_LINE_WIDTH,
      // VK_DYNAMIC_STATE_DEPTH_BIAS,
      // VK_DYNAMIC_STATE_BLEND_CONSTANTS,
      // VK_DYNAMIC_STATE_DEPTH_BOUNDS,
      // VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
      // VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
      // VK_DYNAMIC_STATE_STENCIL_REFERENCE,
  };

  bool dirty_{true};

  friend class GraphicsPipeline;
};
} // namespace mango