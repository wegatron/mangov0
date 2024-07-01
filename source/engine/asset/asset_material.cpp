#include <engine/asset/asset_material.h>
#include <engine/functional/global/engine_context.h>
#include <engine/functional/global/resource_binding_mgr.h>
#include <engine/utils/vk/buffer.h>
#include <engine/utils/vk/image.h>
#include <engine/utils/vk/resource_cache.h>
#include <engine/utils/vk/sampler.h>

namespace mango {
void Material::inflate() {
  auto resource_binding_mgr = g_engine.getResourceBindingMgr();
  auto [des_set, material_buffer, offset] =
      resource_binding_mgr->requestStandardMaterial();
  descriptor_set_ = des_set;
  material_buffer_ = material_buffer;
  offset_ = offset;
  material_buffer_->update(&material_, sizeof(UMaterial), offset_);

  VkDescriptorBufferInfo desc_buffer_info{.buffer =
                                              material_buffer_->getHandle(),
                                          .offset = offset_,
                                          .range = sizeof(UMaterial)};
  auto driver = g_engine.getDriver();
  auto sampler =
      g_engine.getResourceCache()
          ->requestSampler(driver, VkFilter::VK_FILTER_LINEAR,
                           VkFilter::VK_FILTER_LINEAR,
                           VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_LINEAR,
                           VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT,
                           VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT)
          ->getHandle();

  VkDescriptorImageInfo desc_image_infos[] = {
      {.sampler = sampler,
       .imageView = albedo_texture_ == nullptr
                        ? VK_NULL_HANDLE
                        : albedo_texture_->getImageView()->getHandle(),
       .imageLayout =
           VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL}, // binding point 1
      {.sampler = sampler,
       .imageView = normal_texture_ == nullptr
                        ? VK_NULL_HANDLE
                        : normal_texture_->getImageView()->getHandle(),
       .imageLayout =
           VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL}, // binding point 2
      {.sampler = sampler,
       .imageView = emissive_texture_ == nullptr
                        ? VK_NULL_HANDLE
                        : emissive_texture_->getImageView()->getHandle(),
       .imageLayout =
           VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL}, // binding point 3
      {.sampler = sampler,
       .imageView = metallic_roughness_occlution_texture_ == nullptr
                        ? VK_NULL_HANDLE
                        : metallic_roughness_occlution_texture_->getImageView()
                              ->getHandle(),
       .imageLayout =
           VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL} // binding point 4
  };

  driver->update({
      VkWriteDescriptorSet{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                           .dstSet = descriptor_set_->getHandle(),
                           .dstBinding = 0,
                           .descriptorCount = 1,
                           .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                           .pBufferInfo = &desc_buffer_info},
      VkWriteDescriptorSet{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                           .dstSet = descriptor_set_->getHandle(),
                           .dstBinding = 1,
                           .descriptorCount = 4,
                           .descriptorType =
                               VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                           .pImageInfo = desc_image_infos},
  });
}
} // namespace mango