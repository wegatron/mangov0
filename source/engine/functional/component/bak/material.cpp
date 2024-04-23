#include "material.h"
#include <engine/functional/global/app_context.h>
#include <engine/utils/vk/buffer.h>
#include <engine/utils/vk/image.h>
#include <engine/utils/vk/pipeline.h>
#include <engine/utils/vk/resource_cache.h>
#include <engine/utils/vk/sampler.h>

namespace mango {

#define MAX_FORWARD_LIGHT_COUNT 4

MatGpuResourcePool::MatGpuResourcePool(VkFormat color_format,
                                       VkFormat ds_format) {
  auto &driver = getDefaultAppContext().driver;
  VkDescriptorPoolSize pool_size[] = {
      {.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
       .descriptorCount = MAX_MAT_DESC_SET * CONFIG_UNIFORM_BINDING_COUNT},
      {.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
       .descriptorCount = MAX_MAT_DESC_SET * MAT_TEXTURE_NUM_COUNT}};
  desc_pool_ = std::make_unique<DescriptorPool>(
      driver, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT, pool_size,
      sizeof(pool_size) / sizeof(VkDescriptorPoolSize), MAX_MAT_DESC_SET);
  auto &rs_cache = getDefaultAppContext().resource_cache;
  std::vector<Attachment> attachments{
      Attachment{color_format, VK_SAMPLE_COUNT_1_BIT,
                 VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT},
      Attachment{ds_format, VK_SAMPLE_COUNT_1_BIT,
                 VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT}};
  std::vector<LoadStoreInfo> load_store_infos{
      {VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE},
      {VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE}};
  std::vector<SubpassInfo> subpass_infos{{
      {},  // no input attachment
      {0}, // color attachment index
      {},  // no msaa
      1    // depth stencil attachment index
  }};
  default_render_pass_ = rs_cache->requestRenderPass(
      driver, attachments, load_store_infos, subpass_infos);
}

void MatGpuResourcePool::gc() {
  for (auto itr = used_mat_params_set_.begin();
       itr != used_mat_params_set_.end();) {
    if (itr->use_count() == 1) {
      free_mat_params_set_.push_back(*itr);
      itr = used_mat_params_set_.erase(itr);
    } else
      ++itr;
  }
}

std::shared_ptr<GraphicsPipeline> MatGpuResourcePool::requestGraphicsPipeline(
    const std::shared_ptr<Material> &mat) {
  auto itr = mat_pipelines_.find(mat->materialTypeId());
  if (itr != mat_pipelines_.end()) {
    return itr->second;
  }

  // pipeline cache
  auto &driver = getDefaultAppContext().driver;
  auto &rs_cache = getDefaultAppContext().resource_cache;
  auto pipeline_state = std::make_unique<GPipelineState>();
  mat->setPipelineState(*pipeline_state);
  // set other pipeline state:

  auto pipeline = std::make_shared<GraphicsPipeline>(
      driver, rs_cache, default_render_pass_, std::move(pipeline_state));
  mat_pipelines_.emplace(mat->materialTypeId(), pipeline);
  return pipeline;
}

std::shared_ptr<DescriptorSet> MatGpuResourcePool::requestMatDescriptorSet(
    const std::shared_ptr<Material> &mat) {
  if (mat->mat_param_set_ != nullptr)
    return mat->mat_param_set_->desc_set;

  // find a free mat param set
  auto mat_id = mat->materialTypeId();
  auto itr =
      std::find_if(free_mat_params_set_.begin(), free_mat_params_set_.end(),
                   [mat_id](const std::shared_ptr<MatParamsSet> &ps) {
                     return ps->mat_type_id == mat_id;
                   });

  if (itr != free_mat_params_set_.end()) {
    mat->mat_param_set_ = *itr;
    used_mat_params_set_.push_back(*itr);
    free_mat_params_set_.erase(itr);
    mat->updateParams(); // update mat paramsto gpu
    return mat->mat_param_set_->desc_set;
  }

  // create new one
  auto &driver = getDefaultAppContext().driver;
  auto &rs_cache = getDefaultAppContext().resource_cache;
  auto mat_param_set = mat->createMatParamsSet(driver, *desc_pool_);
  mat->mat_param_set_ = mat_param_set;
  used_mat_params_set_.push_back(mat_param_set);
  mat->updateParams(); // update mat paramsto gpu
  return mat_param_set->desc_set;
}

void Material::updateParams() {
  // update uniform buffer params
  if (mat_param_set_ == nullptr)
    return;
  auto &ubo = mat_param_set_->ubo;
  if (ubo_info_.dirty) {
    ubo->update(ubo_info_.data.data(), ubo_info_.size, 0);
    ubo_info_.dirty = false;
  }

  // update textures... descriptor set
  assert(mat_param_set_->desc_set != nullptr);

  std::vector<VkDescriptorImageInfo> desc_img_infos;
  desc_img_infos.reserve(texture_params_.size());

  std::vector<VkWriteDescriptorSet> wds;
  wds.reserve(texture_params_.size());
  for (auto &tp : texture_params_) {
    if (!tp.dirty)
      continue;
    tp.dirty = false;
    // update descriptor set
    auto driver = getDefaultAppContext().driver;
    // save sampler to texture params. to make sure sampler not deconstruct when
    // use
    tp.sampler = getDefaultAppContext().resource_cache->requestSampler(
        driver, VK_FILTER_LINEAR, VK_FILTER_LINEAR,
        VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT,
        VK_SAMPLER_ADDRESS_MODE_REPEAT);

    desc_img_infos.emplace_back(VkDescriptorImageInfo{
        .sampler = tp.sampler->getHandle(),
        .imageView = tp.img_view->getHandle(),
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    });
    wds.emplace_back(VkWriteDescriptorSet{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = mat_param_set_->desc_set->getHandle(),
        .dstBinding = tp.binding,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &desc_img_infos.back()});
  }
  if (!wds.empty())
    getDefaultAppContext().driver->update(wds);
}

// void Material::writeDescriptorSets(
//     VkDescriptorSet descriptor_set) {
//   std::vector<VkWriteDescriptorSet> wds;
//   wds.reserve(ubos_.size());
//   std::vector<VkDescriptorBufferInfo> desc_buffer_infos;
//   desc_buffer_infos.reserve(ubos_.size());
//   for (auto i = 0; i < ubos_.size(); ++i) {
//     desc_buffer_infos.emplace_back(VkDescriptorBufferInfo{
//         .buffer = ubos_[i]->getHandle(),
//         .offset = 0,
//         .range = ubos_info_[i].size,
//     });
//     wds.emplace_back(VkWriteDescriptorSet{
//         .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
//         .dstSet = descriptor_set,
//         .dstBinding = ubos_info_[i].binding,
//         .descriptorCount = 1,
//         .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
//         .pBufferInfo = &desc_buffer_infos.back(),
//     });
//   }

//   if (wds.empty())
//     throw std::runtime_error("no ubo to update");

//   driver_->update(wds);
// }

} // namespace mango