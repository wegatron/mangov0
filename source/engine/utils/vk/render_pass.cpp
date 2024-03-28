#include <cassert>

#include <engine/utils/base/error.h>
#include <engine/utils/vk/render_pass.h>
#include <engine/utils/vk/vk_driver.h>
#include <glm/gtx/hash.hpp>

namespace mango {

size_t LoadStoreInfo::getHash() const {
  std::hash<int> hasher;
  size_t hash = hasher(static_cast<int>(load_op));
  glm::detail::hash_combine(hash, hasher(static_cast<int>(store_op)));
  return hash;
}

size_t Attachment::getHash() const {
  std::hash<int> hasher;
  size_t hash = hasher(static_cast<int>(format));
  glm::detail::hash_combine(hash, hasher(static_cast<int>(samples)));
  glm::detail::hash_combine(hash, hasher(static_cast<int>(usage)));
  glm::detail::hash_combine(hash, hasher(static_cast<int>(initial_layout)));
  return hash;
}

size_t SubpassInfo::getHash() const {
  size_t hash = 0;
  std::hash<int> hasher;
  for (const auto &input_attachment : input_attachments) {
    glm::detail::hash_combine(hash, hasher(input_attachment));
  }
  for (const auto &output_attachment : output_attachments) {
    glm::detail::hash_combine(hash, hasher(output_attachment));
  }
  for (const auto &resolve_attachment : color_resolve_attachments) {
    glm::detail::hash_combine(hash, hasher(resolve_attachment));
  }
  if (depth_stencil_attachment) {
    glm::detail::hash_combine(hash, hasher(depth_stencil_attachment));
  }
  return hash;
}

bool is_depth_only_format(VkFormat format) {
  return format == VK_FORMAT_D16_UNORM || format == VK_FORMAT_D32_SFLOAT;
}

bool is_depth_stencil_format(VkFormat format) {
  return format == VK_FORMAT_D16_UNORM_S8_UINT ||
         format == VK_FORMAT_D24_UNORM_S8_UINT ||
         format == VK_FORMAT_D32_SFLOAT_S8_UINT || is_depth_only_format(format);
}

void fill_input_attachment_refs(
    const std::vector<uint32_t> &attachment_inds,
    const std::vector<VkAttachmentDescription> &attachment_descriptions,
    size_t &ref_ind, std::vector<VkAttachmentReference> &attachment_refs) {
  for (size_t i = 0; i < attachment_inds.size(); ++i) {
    attachment_refs[ref_ind].attachment = attachment_inds[i];
    attachment_refs[ref_ind].layout =
        is_depth_stencil_format(
            attachment_descriptions[attachment_inds[i]].format)
            ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
            : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    ++ref_ind;
  }
}

void fill_output_attachment_refs(
    const std::vector<uint32_t> &attachment_inds,
    const std::vector<VkAttachmentDescription> &attachment_descriptions,
    size_t &ref_ind, std::vector<VkAttachmentReference> &attachment_refs) {
  for (size_t i = 0; i < attachment_inds.size(); ++i) {
    attachment_refs[ref_ind].attachment = attachment_inds[i];
    attachment_refs[ref_ind].layout =
        (attachment_descriptions[attachment_inds[i]].initialLayout ==
         VK_IMAGE_LAYOUT_UNDEFINED)
            ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
            : attachment_descriptions[attachment_inds[i]].initialLayout;
    ++ref_ind;
  }
}

RenderPass::RenderPass(const std::shared_ptr<VkDriver> &driver,
                       const std::vector<Attachment> &attachments,
                       const std::vector<LoadStoreInfo> &load_store_infos,
                       const std::vector<SubpassInfo> &subpasses)
    : driver_(driver) {
  std::vector<VkAttachmentDescription> attachment_descriptions(
      attachments.size());
  for (size_t i = 0; i < attachments.size(); ++i) {
    attachment_descriptions[i].flags = 0;
    // renderpass compatibility options
    attachment_descriptions[i].format = attachments[i].format;
    attachment_descriptions[i].samples = attachments[i].samples;

    // allow different between compatible render passes
    attachment_descriptions[i].loadOp = load_store_infos[i].load_op;
    attachment_descriptions[i].storeOp = load_store_infos[i].store_op;
    attachment_descriptions[i].stencilLoadOp = load_store_infos[i].load_op;
    attachment_descriptions[i].stencilStoreOp = load_store_infos[i].store_op;
    attachment_descriptions[i].initialLayout = attachments[i].initial_layout;
    attachment_descriptions[i].finalLayout =
        is_depth_stencil_format(attachments[i].format)
            ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
            : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  }

  size_t attachment_num = 0;
  for (size_t i = 0; i < subpasses.size(); ++i) {
    attachment_num += subpasses[i].input_attachments.size();
    attachment_num += subpasses[i].output_attachments.size();
    attachment_num += subpasses[i].color_resolve_attachments.size();
    if (subpasses[i].depth_stencil_attachment != 0xFFFFFFFF)
      ++attachment_num;
  }
  size_t ref_ind = 0;
  std::vector<VkAttachmentReference> attachment_refs(attachment_num);
  std::vector<VkSubpassDescription> subpass_descriptions(subpasses.size());
  for (size_t i = 0; i < subpasses.size(); ++i) {
    subpass_descriptions[i].flags = 0;
    subpass_descriptions[i].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_descriptions[i].inputAttachmentCount =
        subpasses[i].input_attachments.size();
    subpass_descriptions[i].pInputAttachments =
        attachment_refs.data() + ref_ind;
    fill_input_attachment_refs(subpasses[i].input_attachments,
                               attachment_descriptions, ref_ind,
                               attachment_refs);

    subpass_descriptions[i].colorAttachmentCount =
        subpasses[i].output_attachments.size();
    subpass_descriptions[i].pColorAttachments =
        attachment_refs.data() + ref_ind;
    fill_output_attachment_refs(subpasses[i].output_attachments,
                                attachment_descriptions, ref_ind,
                                attachment_refs);

    subpass_descriptions[i].pResolveAttachments = nullptr;
    if (subpasses[i].color_resolve_attachments.size() > 0) {
      subpass_descriptions[i].pResolveAttachments =
          attachment_refs.data() + ref_ind;
      fill_output_attachment_refs(subpasses[i].color_resolve_attachments,
                                  attachment_descriptions, ref_ind,
                                  attachment_refs);
    }

    subpass_descriptions[i].pDepthStencilAttachment = nullptr;
    if (subpasses[i].depth_stencil_attachment != 0xFFFFFFFF) {
      subpass_descriptions[i].pDepthStencilAttachment =
          attachment_refs.data() + ref_ind;
      attachment_refs[ref_ind].attachment =
          subpasses[i].depth_stencil_attachment;
      attachment_refs[ref_ind].layout =
          (attachment_descriptions[subpasses[i].depth_stencil_attachment]
               .initialLayout == VK_IMAGE_LAYOUT_UNDEFINED)
              ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
              : attachment_descriptions[subpasses[i].depth_stencil_attachment]
                    .initialLayout;
      ++ref_ind;
    }
  }

  // default subpass dependencies
  std::vector<VkSubpassDependency> subpass_dependencies(subpasses.size() - 1);
  for (size_t i = 0; i < subpasses.size() - 1; ++i) {
    subpass_dependencies[i].srcSubpass = i;
    subpass_dependencies[i].dstSubpass = i + 1;
    subpass_dependencies[i].srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependencies[i].dstStageMask =
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    subpass_dependencies[i].srcAccessMask =
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpass_dependencies[i].dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
    subpass_dependencies[i].dependencyFlags =
        VK_DEPENDENCY_BY_REGION_BIT; // only access samples that match the pixel
  }

  VkRenderPassCreateInfo render_pass_create_info = {
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .attachmentCount = static_cast<uint32_t>(attachment_descriptions.size()),
      .pAttachments = attachment_descriptions.data(),
      .subpassCount = static_cast<uint32_t>(subpass_descriptions.size()),
      .pSubpasses = subpass_descriptions.data(),
      .dependencyCount = static_cast<uint32_t>(subpass_dependencies.size()),
      .pDependencies = subpass_dependencies.data()};

  VkResult result = vkCreateRenderPass(
      driver->getDevice(), &render_pass_create_info, nullptr, &handle_);
  if (result != VK_SUCCESS) {
    throw VulkanException(result, "Failed to create render pass");
  }
}

RenderPass::~RenderPass() {
  vkDestroyRenderPass(driver_->getDevice(), handle_, nullptr);
}

} // namespace mango