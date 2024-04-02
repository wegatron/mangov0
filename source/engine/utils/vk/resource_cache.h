#pragma once
#include <cassert>
#include <engine/utils/vk/descriptor_set_layout.h>
#include <engine/utils/vk/pipeline_layout.h>
#include <engine/utils/vk/render_pass.h>
#include <engine/utils/vk/shader_module.h>
#include <mutex>
#include <unordered_map>

namespace mango {
class Sampler;

class VkPipelineCacheWraper {
public:
  VkPipelineCacheWraper(VkDevice device);

  VkPipelineCacheWraper(const VkPipelineCacheWraper &) = delete;
  VkPipelineCacheWraper &operator=(const VkPipelineCacheWraper &) = delete;
  VkPipelineCacheWraper(VkPipelineCacheWraper &&) = delete;

  ~VkPipelineCacheWraper() {
    assert(device_ != VK_NULL_HANDLE);
    vkDestroyPipelineCache(device_, handle_, nullptr);
  }

  VkPipelineCache getHandle() const { return handle_; }

private:
  VkPipelineCache handle_{VK_NULL_HANDLE};
  VkDevice device_{VK_NULL_HANDLE};
};

template <typename T> struct Resource {
  std::shared_ptr<T> data_ptr;
  mutable uint64_t last_accessed;
};

struct ResourceCacheState {
  std::mutex shader_modules_mtx;
  std::unordered_map<size_t, std::shared_ptr<ShaderModule>>
      shader_modules; //!< hash code of shader module(stage, glsl_code) -->
                      //!< ShaderModule

  std::mutex shaders_mtx;
  std::unordered_map<size_t, std::shared_ptr<Shader>>
      shaders; //!< hash code of shader module --> Shader

  std::mutex descriptor_set_layouts_mtx;
  std::unordered_map<size_t, std::shared_ptr<DescriptorSetLayout>>
      descriptor_set_layouts;

  std::mutex pipeline_layouts_mtx;
  std::unordered_map<size_t, std::shared_ptr<PipelineLayout>> pipeline_layouts;

  std::mutex render_pass_mtx;
  std::unordered_map<size_t, std::shared_ptr<RenderPass>> render_passes;

  std::mutex samples_mtx;
  std::unordered_map<size_t, std::shared_ptr<Sampler>> samplers;

  std::unique_ptr<VkPipelineCacheWraper> pipeline_cache;
};

class ResourceCache final {
public:
  ResourceCache() = default;
  ~ResourceCache() = default;

  ResourceCache(const ResourceCache &) = delete;
  ResourceCache &operator=(const ResourceCache &) = delete;

  ResourceCache(ResourceCache &&) = delete;
  ResourceCache &operator=(ResourceCache &&) = delete;

  std::shared_ptr<ShaderModule>
  requestShaderModule(VkShaderStageFlagBits stage,
                      const std::string &glsl_source,
                      const ShaderVariant &variant);

  std::shared_ptr<ShaderModule>
  requestShaderModule(const std::string &file_path,
                      const ShaderVariant &variant);

  std::shared_ptr<Shader>
  requestShader(const std::shared_ptr<VkDriver> &driver,
                const std::shared_ptr<ShaderModule> &shader_module);

  std::shared_ptr<DescriptorSetLayout>
  requestDescriptorSetLayout(const std::shared_ptr<VkDriver> &driver,
                             const size_t set_index,
                             const std::vector<ShaderResource> &resources);

  std::shared_ptr<PipelineLayout> requestPipelineLayout(
      const std::shared_ptr<VkDriver> &driver,
      const std::vector<std::shared_ptr<ShaderModule>> &shader_modules);

  std::shared_ptr<RenderPass>
  requestRenderPass(const std::shared_ptr<VkDriver> &driver,
                    const std::vector<Attachment> &attachments,
                    const std::vector<LoadStoreInfo> &load_store_infos,
                    const std::vector<SubpassInfo> &subpasses);

  std::shared_ptr<Sampler>
  requestSampler(const std::shared_ptr<VkDriver> &driver, VkFilter mag_filter,
                 VkFilter min_filter, VkSamplerMipmapMode mipmap_mode,
                 VkSamplerAddressMode address_mode_u,
                 VkSamplerAddressMode address_mode_v);

  VkPipelineCache getPipelineCache() const {
    return (state_.pipeline_cache == nullptr)
               ? VK_NULL_HANDLE
               : state_.pipeline_cache->getHandle();
  }

  void
  setPipelineCache(std::unique_ptr<VkPipelineCacheWraper> &&pipeline_cache) {
    state_.pipeline_cache = std::move(pipeline_cache);
  }

  void clear();

  void gc();

private:
  ResourceCacheState state_;
  uint64_t current_frame_{0};
};
} // namespace mango