#include <engine/utils/vk/pipeline_state.h>
#include <string.h>
#include <tuple>

bool operator==(const VkVertexInputBindingDescription &lhs,
                const VkVertexInputBindingDescription &rhs) {
  return lhs.binding == rhs.binding && lhs.stride == rhs.stride &&
         lhs.inputRate == rhs.inputRate;
}

bool operator==(const VkVertexInputAttributeDescription &lhs,
                const VkVertexInputAttributeDescription &rhs) {
  return lhs.binding == rhs.binding &&
         lhs.location == rhs.location & lhs.format == rhs.format &&
         lhs.offset == rhs.offset;
}

bool operator==(const VkPipelineColorBlendAttachmentState &lhs,
                const VkPipelineColorBlendAttachmentState &rhs) {
  return lhs.blendEnable == rhs.blendEnable &&
         lhs.srcColorBlendFactor == rhs.srcColorBlendFactor &&
         lhs.dstColorBlendFactor == rhs.dstColorBlendFactor &&
         lhs.colorBlendOp == rhs.colorBlendOp &&
         lhs.srcAlphaBlendFactor == rhs.srcAlphaBlendFactor &&
         lhs.dstAlphaBlendFactor == rhs.dstAlphaBlendFactor &&
         lhs.alphaBlendOp == rhs.alphaBlendOp &&
         lhs.colorWriteMask == rhs.colorWriteMask;
}

namespace mango {
bool operator!=(const VkPipelineColorBlendAttachmentState &lhs,
                const VkPipelineColorBlendAttachmentState &rhs) {
  return lhs.blendEnable != rhs.blendEnable ||
         lhs.srcColorBlendFactor != rhs.srcColorBlendFactor ||
         lhs.dstColorBlendFactor != rhs.dstColorBlendFactor ||
         lhs.colorBlendOp != rhs.colorBlendOp ||
         lhs.srcAlphaBlendFactor != rhs.srcAlphaBlendFactor ||
         lhs.dstAlphaBlendFactor != rhs.dstAlphaBlendFactor ||
         lhs.alphaBlendOp != rhs.alphaBlendOp ||
         lhs.colorWriteMask != rhs.colorWriteMask;
}

bool operator!=(const VkStencilOpState &lhs, const VkStencilOpState &rhs) {
  return lhs.failOp != rhs.failOp || lhs.passOp != rhs.passOp ||
         lhs.depthFailOp != rhs.depthFailOp || lhs.compareOp != rhs.compareOp ||
         lhs.compareMask != rhs.compareMask || lhs.writeMask != rhs.writeMask ||
         lhs.reference != rhs.reference;
}

bool operator!=(const VertexInputState &lhs, const VertexInputState &rhs) {
  return lhs.bindings != rhs.bindings || lhs.attributes != rhs.attributes;
}

bool operator!=(const InputAssemblyState &lhs, const InputAssemblyState &rhs) {
  return lhs.topology != rhs.topology ||
         lhs.primitive_restart_enable != rhs.primitive_restart_enable;
}

bool operator!=(const RasterizationState &lhs, const RasterizationState &rhs) {
  return lhs.depth_clamp_enable != rhs.depth_clamp_enable ||
         lhs.rasterizer_discard_enable != rhs.rasterizer_discard_enable ||
         lhs.polygon_mode != rhs.polygon_mode ||
         lhs.cull_mode != rhs.cull_mode || lhs.front_face != rhs.front_face ||
         lhs.depth_bias_enable != rhs.depth_bias_enable;
}

bool operator!=(const ViewPortState &lhs, const ViewPortState &rhs) {
  return lhs.viewport_count != rhs.viewport_count ||
         lhs.scissor_count != rhs.scissor_count;
}

bool operator!=(const MultisampleState &lhs, const MultisampleState &rhs) {
  return lhs.rasterization_samples != rhs.rasterization_samples ||
         lhs.sample_shading_enable != rhs.sample_shading_enable ||
         lhs.min_sample_shading != rhs.min_sample_shading ||
         lhs.sample_mask != rhs.sample_mask ||
         lhs.alpha_to_coverage_enable != rhs.alpha_to_coverage_enable ||
         lhs.alpha_to_one_enable != rhs.alpha_to_one_enable;
}

bool operator!=(const DepthStencilState &lhs, const DepthStencilState &rhs) {
  return lhs.depth_test_enable != rhs.depth_test_enable ||
         lhs.depth_write_enable != rhs.depth_write_enable ||
         lhs.depth_compare_op != rhs.depth_compare_op ||
         lhs.depth_bounds_test_enable != rhs.depth_bounds_test_enable ||
         lhs.stencil_test_enable != rhs.stencil_test_enable ||
         lhs.front != rhs.front || lhs.back != rhs.back ||
         lhs.min_depth_bounds != rhs.min_depth_bounds ||
         lhs.max_depth_bounds != rhs.max_depth_bounds;
}

bool operator!=(const ColorBlendState &lhs, const ColorBlendState &rhs) {
  return lhs.logic_op_enable != rhs.logic_op_enable ||
         lhs.logic_op != rhs.logic_op || lhs.attachments != rhs.attachments ||
         lhs.blend_constants != rhs.blend_constants;
}

void VertexInputState::getCreateInfo(
    VkPipelineVertexInputStateCreateInfo &create_info) const {
  create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  create_info.pNext = nullptr;
  create_info.flags = 0;
  create_info.vertexBindingDescriptionCount =
      static_cast<uint32_t>(bindings.size());
  create_info.pVertexBindingDescriptions = bindings.data();
  create_info.vertexAttributeDescriptionCount =
      static_cast<uint32_t>(attributes.size());
  create_info.pVertexAttributeDescriptions = attributes.data();
}

void InputAssemblyState::getCreateInfo(
    VkPipelineInputAssemblyStateCreateInfo &create_info) const {
  create_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  create_info.pNext = nullptr;
  create_info.flags = 0;
  create_info.topology = topology;
  create_info.primitiveRestartEnable = primitive_restart_enable;
}

void RasterizationState::getCreateInfo(
    VkPipelineRasterizationStateCreateInfo &create_info) const {
  create_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  create_info.pNext = nullptr;
  create_info.flags = 0;
  create_info.depthClampEnable = depth_clamp_enable;
  create_info.rasterizerDiscardEnable = rasterizer_discard_enable;
  create_info.polygonMode = polygon_mode;
  create_info.cullMode = cull_mode;
  create_info.frontFace = front_face;
  create_info.depthBiasEnable = depth_bias_enable;
  create_info.depthBiasConstantFactor = 0.0f;
  create_info.depthBiasClamp = depth_clamp_enable ? 1.0f : 0.0f;
  create_info.depthBiasSlopeFactor = 1.0f;
  create_info.lineWidth = 1.0f;
}

void MultisampleState::getCreateInfo(
    VkPipelineMultisampleStateCreateInfo &create_info) const {
  create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  create_info.pNext = nullptr;
  create_info.flags = 0;
  create_info.rasterizationSamples = rasterization_samples;
  create_info.sampleShadingEnable = sample_shading_enable;
  create_info.minSampleShading = min_sample_shading;
  create_info.pSampleMask = &sample_mask;
  create_info.alphaToCoverageEnable = alpha_to_coverage_enable;
  create_info.alphaToOneEnable = alpha_to_one_enable;
}

void DepthStencilState::getCreateInfo(
    VkPipelineDepthStencilStateCreateInfo &create_info) const {
  create_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  create_info.pNext = nullptr;
  create_info.flags = 0;
  create_info.depthTestEnable = depth_test_enable;
  create_info.depthWriteEnable = depth_write_enable;
  create_info.depthCompareOp = depth_compare_op;
  create_info.depthBoundsTestEnable = depth_bounds_test_enable;
  create_info.stencilTestEnable = stencil_test_enable;
  create_info.front = front;
  create_info.back = back;
  create_info.minDepthBounds = min_depth_bounds;
  create_info.maxDepthBounds = max_depth_bounds;
}

void ColorBlendState::getCreateInfo(
    VkPipelineColorBlendStateCreateInfo &create_info) const {
  create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  create_info.pNext = nullptr;
  create_info.flags = 0;
  create_info.logicOpEnable = logic_op_enable;
  create_info.logicOp = logic_op;
  create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
  create_info.pAttachments = attachments.data();
  memcpy(create_info.blendConstants, blend_constants, sizeof(float) * 4);
}

void GPipelineState::setShaderModules(
    const std::vector<std::shared_ptr<ShaderModule>> &shader_modules) {
  if (shader_modules == shader_modules_)
    return;
  shader_modules_ = shader_modules;
  dirty_ = true;
}

void GPipelineState::setVertexInputState(const VertexInputState &state) {
  if (state != vertex_input_state_) {
    vertex_input_state_ = state;
    dirty_ = true;
  }
}

void GPipelineState::setInputAssemblyState(const InputAssemblyState &state) {
  if (state != input_assembly_state_) {
    input_assembly_state_ = state;
    dirty_ = true;
  }
}

void GPipelineState::setRasterizationState(const RasterizationState &state) {
  if (state != rasterization_state_) {
    rasterization_state_ = state;
    dirty_ = true;
  }
}

void GPipelineState::setViewportState(const ViewPortState &state) {
  if (state != viewport_state_) {
    viewport_state_ = state;
    dirty_ = true;
  }
}

void GPipelineState::setMultisampleState(const MultisampleState &state) {
  if (state != multisample_state_) {
    multisample_state_ = state;
    dirty_ = true;
  }
}

void GPipelineState::setDepthStencilState(const DepthStencilState &state) {
  if (state != depth_stencil_state_) {
    depth_stencil_state_ = state;
    dirty_ = true;
  }
}

void GPipelineState::setColorBlendState(const ColorBlendState &state) {
  if (state != color_blend_state_) {
    color_blend_state_ = state;
    dirty_ = true;
  }
}

void GPipelineState::setSubpassIndex(uint32_t subpass_index) {
  if (subpass_index == subpass_index_)
    return;
  subpass_index_ = subpass_index;
  dirty_ = true;
}

void GPipelineState::getDynamicStateCreateInfo(
    VkPipelineDynamicStateCreateInfo &dynamic_state) const {
  dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_state.pDynamicStates = dynamic_states_.data();
  dynamic_state.dynamicStateCount = dynamic_states_.size();
  dynamic_state.flags = 0;
}

} // namespace mango