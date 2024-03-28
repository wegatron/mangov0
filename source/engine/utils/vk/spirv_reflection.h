#pragma once

#include <engine/utils/vk/shader_module.h>
#include <spirv_cross.hpp>
#include <vector>
#include <vulkan/vulkan.h>

namespace mango {
class SPIRVReflection {
public:
  /// @brief Reflects shader resources from SPIRV code
  /// @param stage The Vulkan shader stage flag
  /// @param spirv The SPIRV code of shader
  /// @param[out] resources The list of reflected shader resources
  /// @param variant ShaderVariant used for reflection to specify the size of
  /// the runtime arrays in Storage Buffers
  bool reflect_shader_resources(VkShaderStageFlagBits stage,
                                const std::vector<uint32_t> &spirv,
                                std::vector<ShaderResource> &resources);

private:
  void parse_shader_resources(const spirv_cross::Compiler &compiler,
                              VkShaderStageFlagBits stage,
                              std::vector<ShaderResource> &resources);

  void parse_push_constants(const spirv_cross::Compiler &compiler,
                            VkShaderStageFlagBits stage,
                            std::vector<ShaderResource> &resources);
};
} // namespace mango