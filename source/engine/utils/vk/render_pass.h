#pragma once

#include <memory>
#include <vector>
#include <volk.h>

namespace mango {

class VkDriver;

struct LoadStoreInfo {
  VkAttachmentLoadOp load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
  VkAttachmentStoreOp store_op = VK_ATTACHMENT_STORE_OP_STORE;

  size_t getHash() const;
};

struct Attachment {
  VkFormat format{VK_FORMAT_UNDEFINED};

  VkSampleCountFlagBits samples{VK_SAMPLE_COUNT_1_BIT};

  VkImageUsageFlags usage{VK_IMAGE_USAGE_SAMPLED_BIT};

  VkImageLayout initial_layout{VK_IMAGE_LAYOUT_UNDEFINED};

  size_t getHash() const;
};

struct SubpassInfo {
  std::vector<uint32_t> input_attachments;

  std::vector<uint32_t> output_attachments; // color attachments output

  std::vector<uint32_t> color_resolve_attachments; // for MSAA

  uint32_t depth_stencil_attachment{0xFFFFFFFF};

  size_t getHash() const;

  // uint32_t depth_stencil_resolve_attachment{0xFFFFFFFF};
  // VkResolveModeFlagBits depth_stencil_resolve_mode{VK_RESOLVE_MODE_NONE};
  // std::string debug_name;
};

class RenderPass final {
public:
  RenderPass(const std::shared_ptr<VkDriver> &driver,
             const std::vector<Attachment> &attachments,
             const std::vector<LoadStoreInfo> &load_store_infos,
             const std::vector<SubpassInfo> &subpasses);

  virtual ~RenderPass();

  RenderPass(const RenderPass &) = delete;
  RenderPass &operator=(const RenderPass &) = delete;

  VkRenderPass getHandle() const { return handle_; }

private:
  std::shared_ptr<VkDriver> driver_;
  VkRenderPass handle_{VK_NULL_HANDLE};
};
} // namespace mango