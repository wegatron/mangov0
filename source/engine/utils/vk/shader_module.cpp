#include "shader_module.h"
#include "DirStackFileIncluder.h"
#include <cassert>
#include <fstream>
#include <functional>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GLSL.std.450.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <map>
#include <volk.h>

#include <engine/utils/base/hash_combine.h>
#include <engine/utils/base/macro.h>
#include <engine/utils/vk/spirv_reflection.h>

namespace mango {

size_t ShaderResource::hash(const ShaderResource &resource) noexcept {
  size_t hash_code = 0;
  if (resource.type == ShaderResourceType::Input ||
      resource.type == ShaderResourceType::Output ||
      resource.type == ShaderResourceType::PushConstant ||
      resource.type == ShaderResourceType::SpecializationConstant) {
    return 0;
  }

  std::hash<int> hasher;
  // hash resource part for descriptor binding, no location, location is used
  // for shader input output
  hash_code = hasher(resource.set);
  hash_combine(hash_code, hasher(resource.binding));
  hash_combine(hash_code, hasher(static_cast<int>(resource.type)));
  hash_combine(hash_code, hasher(static_cast<int>(resource.mode)));
  return hash_code;
}

void ShaderVariant::addDefinitions(
    const std::vector<std::string> &definitions) {
  for (const auto &def : definitions)
    addDefine(def);
}

void ShaderVariant::addDefine(const std::string &def) {
  preamble_ += "#define " + def + "\n";
}

void ShaderVariant::addUndefine(const std::string &undef) {
  preamble_ += "#undef " + undef + "\n";
}

void ShaderModule::load(const std::string &file_path) {
  readGlsl(file_path, stage_, glsl_code_);
  setGlsl(glsl_code_, stage_);
}

void ShaderModule::setGlsl(const std::string &glsl_code,
                           VkShaderStageFlagBits stage) {

  glsl_code_ = glsl_code;
  stage_ = stage;

  compile2spirv(glsl_code_, variant_.getPreamble(), stage_, spirv_code_);

  // update shader resources
  SPIRVReflection spirv_reflection;

  // Reflect all shader resouces
  if (!spirv_reflection.reflect_shader_resources(stage_, spirv_code_,
                                                 resources_)) {
    throw std::runtime_error("Failed to reflect shader resources");
  }

  // update hash code
  hash_code_ = hash(glsl_code_, stage_);
}

EShLanguage findShaderLanguage(VkShaderStageFlagBits stage);

void ShaderModule::compile2spirv(const std::string &glsl_code,
                                 const std::string &preamble,
                                 VkShaderStageFlagBits stage,
                                 std::vector<uint32_t> &spirv_code) {
  // if(stage == VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT)
  // {
  //   std::ifstream inputFile("debug/debug_frag.spv", std::ifstream::binary);
  //   inputFile.seekg(0, std::ios::end);
  //   std::streampos fileSize = inputFile.tellg();
  //   inputFile.seekg(0, std::ios::beg);
  //   spirv_code.resize(fileSize / 4);
  //   inputFile.read(reinterpret_cast<char*>(spirv_code.data()), fileSize);

  //   // Close the file after reading
  //   inputFile.close();
  //   return;
  // } else if(stage == VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT)
  // {
  //   std::ifstream inputFile("debug/debug_vert.spv", std::ifstream::binary);
  //   inputFile.seekg(0, std::ios::end);
  //   std::streampos fileSize = inputFile.tellg();
  //   inputFile.seekg(0, std::ios::beg);
  //   spirv_code.resize(fileSize / 4);
  //   inputFile.read(reinterpret_cast<char*>(spirv_code.data()), fileSize);

  //   // Close the file after reading
  //   inputFile.close();
  //   return;
  // }
  // Initialize glslang library.
  glslang::InitializeProcess();

  DirStackFileIncluder includer;
  includer.pushExternalLocalDirectory("shaders/include");

  EShLanguage lang = findShaderLanguage(stage);
  glslang::TShader shader(lang);
  const char *file_name_list[1] = {""};
  const char *shader_source = glsl_code.data();
  shader.setStringsWithLengthsAndNames(&shader_source, nullptr, file_name_list,
                                       1);
  shader.setEntryPoint("main");
  shader.setSourceEntryPoint("main");
  shader.setPreamble(preamble.c_str());
  // shader.setEnvClient(glslang::EShClient::EShClientVulkan,
  //                     glslang::EShTargetClientVersion::EShTargetVulkan_1_3);
  // shader.setEnvTarget(glslang::EShTargetLanguage::EShTargetSpv,
  // glslang::EShTargetLanguageVersion::EShTargetSpv_1_6);
  // shader.setDebugInfo(true);
  // EShMsgDebugInfo for debug in renderdoc
  EShMessages messages = static_cast<EShMessages>(
      EShMsgVulkanRules | EShMsgSpvRules | EShMsgDebugInfo);
  if (!shader.parse(GetDefaultResources(), 100, false, messages,
                    includer)) // 110 for desktop, 100 for es
  {
    auto error_msg = std::string(shader.getInfoLog()) + "\n" +
                     std::string(shader.getInfoDebugLog());
    throw std::runtime_error("compile glsl to spirv error: " + error_msg);
  }
  // Add shader to new program object.
  glslang::TProgram program;
  program.addShader(&shader);

  if (!program.link(messages)) {
    auto error_msg = std::string(program.getInfoLog()) + "\n" +
                     std::string(program.getInfoDebugLog());
    throw std::runtime_error("link program error: " + error_msg);
  }

  auto shader_log = shader.getInfoLog();
  if (strlen(shader_log) > 0)
    LOGI(shader_log);
  auto program_log = program.getInfoLog();
  if (strlen(program_log))
    LOGI(program_log);

  glslang::TIntermediate *intermediate = program.getIntermediate(lang);
  if (!intermediate) {
    throw std::runtime_error("failed to get shader intermediate code");
  }

  spv::SpvBuildLogger logger;

// to enable optimization, add option
// debug in renderdoc need vulkan 1.3
// refer to:
// https://www.khronos.org/assets/uploads/developers/presentations/Source-level_Shader_Debugging_in_Vulkan_with_RenderDoc_VULOCT2022.pdf
#ifndef NDEBUG
  glslang::SpvOptions options;
  options.generateDebugInfo = true;
  // options.stripDebugInfo = false;
  options.disableOptimizer = true;
  // options.optimizeSize = false;
  options.disassemble = true;
  options.validate = true;
  options.emitNonSemanticShaderDebugInfo = true;
  options.emitNonSemanticShaderDebugSource = true;
  glslang::GlslangToSpv(*intermediate, spirv_code, &logger, &options);
#else
  glslang::GlslangToSpv(*intermediate, spirv_code, &logger);
#endif
  auto log_str = logger.getAllMessages();
  if (log_str.length() > 0)
    LOGI(logger.getAllMessages());
  glslang::FinalizeProcess();
}

void ShaderModule::readGlsl(const std::string &file_path,
                            VkShaderStageFlagBits &stage,
                            std::string &glsl_code) {
  auto len = file_path.length();
  if (len <= 4)
    throw std::runtime_error("invalid shader file path");

  if (file_path.compare(len - 5, 5, ".vert") == 0) {
    stage = VK_SHADER_STAGE_VERTEX_BIT;
  } else if (file_path.compare(len - 5, 5, ".frag") == 0) {
    stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  } else if (file_path.compare(len - 5, 5, ".comp") == 0) {
    stage = VK_SHADER_STAGE_COMPUTE_BIT;
  } else {
    throw std::runtime_error(
        "invalid shader file path post fix, only support .vert, .frag, .comp");
  }

  // read glsl code
  std::ifstream ifs(file_path, std::ifstream::binary);
  if (!ifs)
    throw std::runtime_error("can't open file " + file_path);
  ifs.seekg(0, std::ios::end);
  size_t size = ifs.tellg();
  ifs.seekg(0, std::ios::beg);
  glsl_code.resize(size);
  ifs.read(reinterpret_cast<char *>(glsl_code.data()), size);
  ifs.close();
}

size_t ShaderModule::hash(const std::string &glsl_code,
                          VkShaderStageFlagBits stage) noexcept {
  auto hash_code = std::hash<VkShaderStageFlagBits>{}(stage);
  auto value = std::hash<std::string>{}(glsl_code);
  hash_combine(hash_code, value);
  return hash_code;
}

Shader::Shader(const std::shared_ptr<VkDriver> &driver,
               const std::shared_ptr<ShaderModule> &shader_module) {
  assert(shader_module != nullptr);

  driver_ = driver;
  VkShaderModuleCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  create_info.codeSize = shader_module->getSpirv().size() * sizeof(uint32_t);
  create_info.pCode = shader_module->getSpirv().data();

  if (vkCreateShaderModule(driver_->getDevice(), &create_info, nullptr,
                           &handle_) != VK_SUCCESS) {
    throw std::runtime_error("failed to create shader module!");
  }
}

Shader::~Shader() {
  vkDestroyShaderModule(driver_->getDevice(), handle_, nullptr);
}

// helper functions
EShLanguage findShaderLanguage(VkShaderStageFlagBits stage) {
  switch (stage) {
  case VK_SHADER_STAGE_VERTEX_BIT:
    return EShLangVertex;

  case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
    return EShLangTessControl;

  case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
    return EShLangTessEvaluation;

  case VK_SHADER_STAGE_GEOMETRY_BIT:
    return EShLangGeometry;

  case VK_SHADER_STAGE_FRAGMENT_BIT:
    return EShLangFragment;

  case VK_SHADER_STAGE_COMPUTE_BIT:
    return EShLangCompute;

  case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
    return EShLangRayGen;

  case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:
    return EShLangAnyHit;

  case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
    return EShLangClosestHit;

  case VK_SHADER_STAGE_MISS_BIT_KHR:
    return EShLangMiss;

  case VK_SHADER_STAGE_INTERSECTION_BIT_KHR:
    return EShLangIntersect;

  case VK_SHADER_STAGE_CALLABLE_BIT_KHR:
    return EShLangCallable;

  default:
    return EShLangVertex;
  }
}

std::vector<ShaderResource> parseShaderResources(
    const std::vector<std::shared_ptr<ShaderModule>> &shader_modules) {
  std::map<std::string, ShaderResource> resources;
  for (const auto shader_module : shader_modules) {
    for (const auto &resource : shader_module->getResources()) {
      std::string key = resource.name;
      // input output may have same name
      if (resource.type == ShaderResourceType::Input ||
          resource.type == ShaderResourceType::Output) {
        key = std::to_string(resource.stages) + "_" + key;
      }

      auto itr = resources.find(key);
      if (itr != resources.end())
        itr->second.stages |= resource.stages;
      else
        resources.emplace(key, resource);
    }
  }

  std::vector<ShaderResource> result;
  result.reserve(resources.size());
  for (auto &resource : resources)
    result.push_back(resource.second);
  return result;
}
} // namespace mango