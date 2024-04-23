#include <engine/utils/base/hash_combine.h>
#include <engine/utils/vk/resource_cache.h>
#include <engine/utils/vk/sampler.h>

namespace mango {

void ResourceCache::init(const std::shared_ptr<VkDriver> &driver) {
  state_.pipeline_cache =
      std::make_unique<VkPipelineCacheWraper>(driver->getDevice());
}

std::shared_ptr<ShaderModule>
ResourceCache::requestShaderModule(VkShaderStageFlagBits stage,
                                   const std::string &glsl_source,
                                   const ShaderVariant &variant) {
  std::unique_lock<std::mutex> lock(state_.shader_modules_mtx);
  auto hash_code = ShaderModule::hash(glsl_source, stage);
  auto iter = state_.shader_modules.find(hash_code);
  if (iter != state_.shader_modules.end())
    return iter->second;

  auto shader_module = std::make_shared<ShaderModule>(variant);
  shader_module->setGlsl(glsl_source, stage);
  state_.shader_modules[hash_code] = shader_module;
  return shader_module;
}

std::shared_ptr<ShaderModule>
ResourceCache::requestShaderModule(const std::string &file_path,
                                   const ShaderVariant &variant) {
  VkShaderStageFlagBits stage{};
  std::string glsl_code;
  ShaderModule::readGlsl(file_path, stage, glsl_code);
  return requestShaderModule(stage, glsl_code, variant);
}

std::shared_ptr<Shader> ResourceCache::requestShader(
    const std::shared_ptr<VkDriver> &driver,
    const std::shared_ptr<ShaderModule> &shader_module) {
  std::unique_lock<std::mutex> lock(state_.shaders_mtx);
  auto hash_code = shader_module->getHash();
  auto iter = state_.shaders.find(hash_code);
  if (iter != state_.shaders.end())
    return iter->second;
  auto shader = std::make_shared<Shader>(driver, shader_module);
  state_.shaders[hash_code] = shader;
  return shader;
}

std::shared_ptr<DescriptorSetLayout> ResourceCache::requestDescriptorSetLayout(
    const std::shared_ptr<VkDriver> &driver, const size_t set_index,
    const std::vector<ShaderResource> &resources) {
  size_t hash_code = 0;
  for (const auto &rs : resources) {
    assert(rs.set == set_index || rs.set == 0XFFFFFFFF);
    auto tmp_hash_code = ShaderResource::hash(rs);
    hash_combine(hash_code, tmp_hash_code);
  }
  std::unique_lock<std::mutex> lock(state_.descriptor_set_layouts_mtx);
  auto itr = state_.descriptor_set_layouts.find(hash_code);
  if (itr != state_.descriptor_set_layouts.end())
    return itr->second;

  auto descriptor_set_layout = std::make_shared<DescriptorSetLayout>(
      driver, set_index, resources.data(), resources.size());
  state_.descriptor_set_layouts[hash_code] = descriptor_set_layout;
  return descriptor_set_layout;
}

std::shared_ptr<PipelineLayout> ResourceCache::requestPipelineLayout(
    const std::shared_ptr<VkDriver> &driver,
    const std::vector<std::shared_ptr<ShaderModule>> &shader_modules) {
  size_t hash_code = 0;
  for (const auto &shader_module : shader_modules) {
    hash_combine(hash_code, shader_module->getHash());
  }
  std::unique_lock<std::mutex> lock(state_.pipeline_layouts_mtx);
  auto itr = state_.pipeline_layouts.find(hash_code);
  if (itr != state_.pipeline_layouts.end())
    return itr->second;

  auto pipeline_layout =
      std::make_shared<PipelineLayout>(driver, shader_modules);
  state_.pipeline_layouts[hash_code] = pipeline_layout;
  return pipeline_layout;
}

std::shared_ptr<RenderPass> ResourceCache::requestRenderPass(
    const std::shared_ptr<VkDriver> &driver,
    const std::vector<Attachment> &attachments,
    const std::vector<LoadStoreInfo> &load_store_infos,
    const std::vector<SubpassInfo> &subpasses) {
  size_t hash_code = 0;
  for (const auto &attachment : attachments) {
    hash_combine(hash_code, attachment.getHash());
  }
  for (const auto &load_store_info : load_store_infos) {
    hash_combine(hash_code, load_store_info.getHash());
  }
  for (const auto &subpass : subpasses) {
    hash_combine(hash_code, subpass.getHash());
  }
  std::unique_lock<std::mutex> lock(state_.render_pass_mtx);
  auto itr = state_.render_passes.find(hash_code);
  if (itr != state_.render_passes.end())
    return itr->second;

  auto render_pass = std::make_shared<RenderPass>(driver, attachments,
                                                  load_store_infos, subpasses);
  state_.render_passes[hash_code] = render_pass;
  return render_pass;
}

std::shared_ptr<Sampler> ResourceCache::requestSampler(
    const std::shared_ptr<VkDriver> &driver, VkFilter mag_filter,
    VkFilter min_filter, VkSamplerMipmapMode mipmap_mode,
    VkSamplerAddressMode address_mode_u, VkSamplerAddressMode address_mode_v) {
  size_t hash_code = 0;
  hash_combine(hash_code, static_cast<size_t>(mag_filter));
  hash_combine(hash_code, static_cast<size_t>(min_filter));
  hash_combine(hash_code, static_cast<size_t>(mipmap_mode));
  hash_combine(hash_code, static_cast<size_t>(address_mode_u));
  hash_combine(hash_code, static_cast<size_t>(address_mode_v));

  std::unique_lock<std::mutex> lock(state_.samples_mtx);
  auto itr = state_.samplers.find(hash_code);
  if (itr != state_.samplers.end()) {
    return itr->second;
  }
  auto s =
      std::make_shared<Sampler>(driver, mag_filter, min_filter, mipmap_mode,
                                address_mode_u, address_mode_v);
  state_.samplers[hash_code] = s;
  return s;
}

void ResourceCache::clear() {
  std::unique_lock<std::mutex> lock(state_.shaders_mtx);
  state_.shaders.clear();

  std::unique_lock<std::mutex> lock2(state_.shader_modules_mtx);
  state_.shader_modules.clear();

  std::unique_lock<std::mutex> lock3(state_.descriptor_set_layouts_mtx);
  state_.descriptor_set_layouts.clear();

  std::unique_lock<std::mutex> lock4(state_.pipeline_layouts_mtx);
  state_.pipeline_layouts.clear();

  std::unique_lock<std::mutex> lock5(state_.render_pass_mtx);
  state_.render_passes.clear();

  std::unique_lock<std::mutex> lock6(state_.samples_mtx);
  state_.samplers.clear();
}

void ResourceCache::gc() {
  if (++current_frame_ < DATA_RESOURCE_TIME_BEFORE_EVICTION)
    return;
  auto data_resource = state_.data_resources;
  for (auto itr = data_resource.begin(); itr != data_resource.end();) {
    auto &a = itr->second;
    if (a.data_ptr.use_count() == 1 &&
        a.last_accessed + DATA_RESOURCE_TIME_BEFORE_EVICTION <=
            current_frame_) {
      itr = data_resource.erase(itr);
    } else
      ++itr;
  }
}

VkPipelineCacheWraper::VkPipelineCacheWraper(VkDevice device)
    : device_(device) {
  VkPipelineCacheCreateInfo create_info{
      VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO};
  vkCreatePipelineCache(device, &create_info, nullptr, &handle_);
}

} // namespace mango
