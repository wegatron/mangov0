#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <engine/utils/vk/vk_constants.h>
#include <engine/utils/vk/vk_driver.h>

namespace mango {

/// A bitmask of qualifiers applied to a resource
struct ShaderResourceQualifiers {
  enum : uint32_t {
    None = 0,
    NonReadable = 1,
    NonWritable = 2,
  };
};

/// Types of shader resources
enum class ShaderResourceType : int {
  Input,
  InputAttachment,
  Output,
  Image,
  ImageSampler,
  ImageStorage,
  Sampler,
  BufferUniform,
  BufferStorage,
  PushConstant,
  SpecializationConstant,
  All
};

/// This determines the type and method of how descriptor set should be created
/// and bound
enum class ShaderResourceMode { Static, Dynamic, UpdateAfterBind };

struct ShaderResource {
  VkShaderStageFlags stages{0};

  ShaderResourceType type;

  ShaderResourceMode mode;

  uint32_t set{0XFFFFFFFF};

  uint32_t binding{0XFFFFFFFF};

  uint32_t location{0XFFFFFFFF};

  uint32_t input_attachment_index{0XFFFFFFFF};

  uint32_t vec_size;

  uint32_t columns;

  uint32_t array_size{1};

  uint32_t offset;

  uint32_t size;

  uint32_t constant_id;

  uint32_t qualifiers;

  std::string name;

  static size_t hash(const ShaderResource &resource) noexcept;
};

class ShaderVariant {
public:
  ShaderVariant() {}

  ShaderVariant(std::string &&preamble) : preamble_(std::move(preamble)) {}

  /**
   * @brief Add definitions to shader variant
   * @param definitions Vector of definitions to add to the variant
   */
  void addDefinitions(const std::vector<std::string> &definitions);

  /**
   * @brief Adds a define macro to the shader
   * @param def String which should go to the right of a define directive
   */
  void addDefine(const std::string &def);

  /**
   * @brief Adds an undef macro to the shader
   * @param undef String which should go to the right of an undef directive
   */
  void addUndefine(const std::string &undef);

  const std::string &getPreamble() const { return preamble_; }

private:
  std::string preamble_;
};

class ShaderModule final {
public:
  ShaderModule(const ShaderVariant &variant) : variant_(variant) {}

  /**
   * @brief load shader file
   * .vert for vertex shader glsl
   * .frag for fragment shader glsl
   * .comp for computer shader glsl
   * @param file_path
   */
  void load(const std::string &file_path);

  /**
   * @brief Set the Glsl object and do precompile
   */
  void setGlsl(const std::string &glsl_code, VkShaderStageFlagBits stage);

  const std::string &getGlsl() const noexcept { return glsl_code_; }

  const std::vector<uint32_t> &getSpirv() const noexcept { return spirv_code_; }

  size_t getHash() const noexcept { return hash_code_; }

  VkShaderStageFlagBits getStage() const noexcept { return stage_; }

  const std::vector<ShaderResource> &getResources() const noexcept {
    return resources_;
  }

  static size_t hash(const std::string &glsl_code,
                     VkShaderStageFlagBits stage) noexcept;

  static void compile2spirv(const std::string &glsl_code,
                            const std::string &preamble,
                            VkShaderStageFlagBits stage,
                            std::vector<uint32_t> &spirv_code);
  static void readGlsl(const std::string &file_path,
                       VkShaderStageFlagBits &stage, std::string &glsl_code);

private:
  size_t hash_code_{0};
  VkShaderStageFlagBits stage_;
  std::shared_ptr<VkDriver> driver_;
  std::string glsl_code_;
  std::vector<uint32_t> spirv_code_;
  std::vector<ShaderResource> resources_;
  ShaderVariant variant_;
};

class Shader final {
public:
  Shader(const std::shared_ptr<VkDriver> &driver,
         const std::shared_ptr<ShaderModule> &shader_module);

  ~Shader();

  VkShaderModule getHandle() const noexcept { return handle_; }

private:
  std::shared_ptr<VkDriver> driver_;
  VkShaderModule handle_;
};

/*
 * @brief parse shader resources, return a vector of ShaderResource, sorted by
 * set index
 */
std::vector<ShaderResource> parseShaderResources(
    const std::vector<std::shared_ptr<ShaderModule>> &shader_modules);
} // namespace mango