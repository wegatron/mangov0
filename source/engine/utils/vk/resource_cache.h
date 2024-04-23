#pragma once
#include <cassert>
#include <engine/functional/global/engine_context.h>
#include <engine/utils/vk/commands.h>
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

struct Resource {
  std::shared_ptr<void> data_ptr;
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

  std::mutex data_resources_mtx;
  std::unordered_map<std::string, Resource>
      data_resources; //!< image views, buffers, etc.
};

class ResourceCache final {
public:
  ResourceCache() = default;
  ~ResourceCache() = default;

  ResourceCache(const ResourceCache &) = delete;
  ResourceCache &operator=(const ResourceCache &) = delete;

  ResourceCache(ResourceCache &&) = delete;
  ResourceCache &operator=(ResourceCache &&) = delete;

  void init(const std::shared_ptr<VkDriver> &driver);

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

  // template <typename T>
  // std::shared_ptr<T> request(const std::string &path,
  //                            const std::shared_ptr<CommandBuffer> &cmd_buf) {
  //   auto &data_resources = state_.data_resources;
  //   auto itr = data_resources.find(path);
  //   if (itr != data_resources.end()) {
  //     return std::static_pointer_cast<T>(itr->second.data_ptr);
  //   }
  //   std::shared_ptr<T> ret;
  //   if (cmd_buf == nullptr) {
  //     auto driver = g_engine.getDriver();
  //     auto ccmd_buf = driver->requestCommandBuffer(
  //         VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY);
  //     ccmd_buf->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
  //     ret = load<T>(path, ccmd_buf);
  //     ccmd_buf->end();
  //     driver->getGraphicsQueue()->submit(ccmd_buf, VK_NULL_HANDLE);
  //     driver->getGraphicsQueue()->waitIdle();
  //   } else {
  //     ret = load<T>(path, cmd_buf);
  //   }
  //   data_resources.emplace(path, Resource{ret, current_frame_});
  //   return ret;
  // }

  // template <typename T>
  // std::shared_ptr<T> request(const uint8_t *data, const size_t size,
  //                            const std::shared_ptr<CommandBuffer> &cmd_buf) {
  //   auto hash_code =
  //       std::to_string(std::hash<std::string>{}(std::string(data, size)));
  //   auto &data_resources = state_.data_resources;
  //   auto itr = data_resources.find(hash_code);
  //   if (itr != data_resources.end()) {
  //     return std::static_pointer_cast<T>(itr->second.data_ptr);
  //   }
  //   std::shared_ptr<T> ret;
  //   if (cmd_buf == nullptr) {
  //     auto driver = g_engine.getDriver();
  //     auto ccmd_buf = driver->requestCommandBuffer(
  //         VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY);
  //     ccmd_buf->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
  //     auto ret = load<T>(data, size, ccmd_buf);
  //     ccmd_buf->end();
  //     driver->getGraphicsQueue()->submit(ccmd_buf, VK_NULL_HANDLE);
  //     driver->getGraphicsQueue()->waitIdle();
  //   } else {
  //     auto ret = load<T>(data, size, cmd_buf);
  //   }
  //   data_resources.emplace(hash_code, Resource{ret, current_frame_});
  //   return ret;
  // }

  // template <typename T>
  // std::shared_ptr<T> request(const float *data, const uint32_t width,
  //                            const uint32_t height, const uint32_t channel,
  //                            const std::shared_ptr<CommandBuffer> &cmd_buf) {
  //   auto hash_code = std::to_string(std::hash<std::string>{}(
  //       std::string(reinterpret_cast<char *>(data),
  //                   width * height * channel * sizeof(float))));
  //   auto &data_resources = state_.data_resources;
  //   auto itr = data_resources.find(hash_code);
  //   if (itr != data_resources.end()) {
  //     return std::static_pointer_cast<T>(itr->second.data_ptr);
  //   }
  //   std::shared_ptr<T> ret;
  //   if (cmd_buf == nullptr) {
  //     auto driver = g_engine.getDriver();
  //     auto ccmd_buf = driver->requestCommandBuffer(
  //         VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY);
  //     ccmd_buf->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
  //     ret = load<T>(data, width, height, channel, ccmd_buf);
  //     ccmd_buf->end();
  //     driver->getGraphicsQueue()->submit(ccmd_buf, VK_NULL_HANDLE);
  //     driver->getGraphicsQueue()->waitIdle();
  //   } else {
  //     ret = load<T>(data, width, height, channel, cmd_buf);
  //   }
  //   data_resources.emplace(hash_code, Resource{ret, current_frame_});
  //   return ret;
  // }

  VkPipelineCache getPipelineCache() const {
    return (state_.pipeline_cache == nullptr)
               ? VK_NULL_HANDLE
               : state_.pipeline_cache->getHandle();
  }

  // void
  // setPipelineCache(std::unique_ptr<VkPipelineCacheWraper> &&pipeline_cache) {
  //   state_.pipeline_cache = std::move(pipeline_cache);
  // }

  void clear();

  void gc();

private:
  ResourceCacheState state_;
  uint64_t current_frame_{0};
};
} // namespace mango