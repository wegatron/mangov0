#include <algorithm>
#include <engine/utils/base/macro.h>
#include <engine/utils/vk/spirv_reflection.h>
#include <limits>
#include <spirv_glsl.hpp>

namespace mango {
template <ShaderResourceType T>
void read_shader_resource(const spirv_cross::Compiler &compiler,
                          VkShaderStageFlagBits stage,
                          std::vector<ShaderResource> &resources) {
  LOGE("Not implemented! Read shader resources of type.");
}

void read_resource_vec_size(const spirv_cross::Compiler &compiler,
                            const spirv_cross::Resource &resource,
                            ShaderResource &shader_resource) {
  const auto &spirv_type = compiler.get_type_from_variable(resource.id);
  shader_resource.vec_size = spirv_type.vecsize;
  shader_resource.columns = spirv_type.columns;
}

void read_resource_array_size(const spirv_cross::Compiler &compiler,
                              const spirv_cross::Resource &resource,
                              ShaderResource &shader_resource) {
  const auto &spirv_type = compiler.get_type_from_variable(resource.id);
  shader_resource.array_size =
      spirv_type.array.size() ? spirv_type.array[0] : 1;
}

template <spv::Decoration T>
void read_resource_decoration(const spirv_cross::Compiler & /*compiler*/,
                              const spirv_cross::Resource & /*resource*/,
                              ShaderResource & /*shader_resource*/) {
  LOGE("Not implemented! Read resources decoration of type.");
}

template <>
void read_resource_decoration<spv::DecorationLocation>(
    const spirv_cross::Compiler &compiler,
    const spirv_cross::Resource &resource, ShaderResource &shader_resource) {
  shader_resource.location =
      compiler.get_decoration(resource.id, spv::DecorationLocation);
}

template <>
void read_resource_decoration<spv::DecorationDescriptorSet>(
    const spirv_cross::Compiler &compiler,
    const spirv_cross::Resource &resource, ShaderResource &shader_resource) {
  shader_resource.set =
      compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
}

template <>
void read_resource_decoration<spv::DecorationBinding>(
    const spirv_cross::Compiler &compiler,
    const spirv_cross::Resource &resource, ShaderResource &shader_resource) {
  shader_resource.binding =
      compiler.get_decoration(resource.id, spv::DecorationBinding);
}

template <>
void read_resource_decoration<spv::DecorationInputAttachmentIndex>(
    const spirv_cross::Compiler &compiler,
    const spirv_cross::Resource &resource, ShaderResource &shader_resource) {
  shader_resource.input_attachment_index =
      compiler.get_decoration(resource.id, spv::DecorationInputAttachmentIndex);
}

template <>
void read_resource_decoration<spv::DecorationNonWritable>(
    const spirv_cross::Compiler &compiler,
    const spirv_cross::Resource &resource, ShaderResource &shader_resource) {
  shader_resource.qualifiers |= ShaderResourceQualifiers::NonWritable;
}

template <>
void read_resource_decoration<spv::DecorationNonReadable>(
    const spirv_cross::Compiler &compiler,
    const spirv_cross::Resource &resource, ShaderResource &shader_resource) {
  shader_resource.qualifiers |= ShaderResourceQualifiers::NonReadable;
}

void read_resource_size(const spirv_cross::Compiler &compiler,
                        const spirv_cross::Resource &resource,
                        ShaderResource &shader_resource) {
  const auto &spirv_type = compiler.get_type_from_variable(resource.id);

  size_t array_size = 0;

  shader_resource.size =
      compiler.get_declared_struct_size_runtime_array(spirv_type, array_size);
}

void read_resource_size(const spirv_cross::Compiler &compiler,
                        const spirv_cross::SPIRConstant &constant,
                        ShaderResource &shader_resource) {
  auto spirv_type = compiler.get_type(constant.constant_type);

  switch (spirv_type.basetype) {
  case spirv_cross::SPIRType::BaseType::Boolean:
  case spirv_cross::SPIRType::BaseType::Char:
  case spirv_cross::SPIRType::BaseType::Int:
  case spirv_cross::SPIRType::BaseType::UInt:
  case spirv_cross::SPIRType::BaseType::Float:
    shader_resource.size = 4;
    break;
  case spirv_cross::SPIRType::BaseType::Int64:
  case spirv_cross::SPIRType::BaseType::UInt64:
  case spirv_cross::SPIRType::BaseType::Double:
    shader_resource.size = 8;
    break;
  default:
    shader_resource.size = 0;
    break;
  }
}

template <>
void read_shader_resource<ShaderResourceType::Input>(
    const spirv_cross::Compiler &compiler, VkShaderStageFlagBits stage,
    std::vector<ShaderResource> &resources) {
  auto input_resources = compiler.get_shader_resources().stage_inputs;

  for (auto &resource : input_resources) {
    ShaderResource shader_resource{};
    shader_resource.type = ShaderResourceType::Input;
    shader_resource.stages = stage;
    shader_resource.name = resource.name;

    read_resource_vec_size(compiler, resource, shader_resource);
    read_resource_array_size(compiler, resource, shader_resource);
    read_resource_decoration<spv::DecorationLocation>(compiler, resource,
                                                      shader_resource);
    resources.push_back(shader_resource);
  }
}

template <>
void read_shader_resource<ShaderResourceType::InputAttachment>(
    const spirv_cross::Compiler &compiler, VkShaderStageFlagBits /*stage*/,
    std::vector<ShaderResource> &resources) {
  auto subpass_resources = compiler.get_shader_resources().subpass_inputs;

  for (auto &resource : subpass_resources) {
    ShaderResource shader_resource{};
    shader_resource.type = ShaderResourceType::InputAttachment;
    shader_resource.stages = VK_SHADER_STAGE_FRAGMENT_BIT;
    shader_resource.name = resource.name;

    read_resource_array_size(compiler, resource, shader_resource);
    read_resource_decoration<spv::DecorationInputAttachmentIndex>(
        compiler, resource, shader_resource);
    read_resource_decoration<spv::DecorationDescriptorSet>(compiler, resource,
                                                           shader_resource);
    read_resource_decoration<spv::DecorationBinding>(compiler, resource,
                                                     shader_resource);

    resources.push_back(shader_resource);
  }
}

template <>
void read_shader_resource<ShaderResourceType::Output>(
    const spirv_cross::Compiler &compiler, VkShaderStageFlagBits stage,
    std::vector<ShaderResource> &resources) {
  auto output_resources = compiler.get_shader_resources().stage_outputs;

  for (auto &resource : output_resources) {
    ShaderResource shader_resource{};
    shader_resource.type = ShaderResourceType::Output;
    shader_resource.stages = stage;
    shader_resource.name = resource.name;

    read_resource_array_size(compiler, resource, shader_resource);
    read_resource_vec_size(compiler, resource, shader_resource);
    read_resource_decoration<spv::DecorationLocation>(compiler, resource,
                                                      shader_resource);

    resources.push_back(shader_resource);
  }
}

template <>
void read_shader_resource<ShaderResourceType::Image>(
    const spirv_cross::Compiler &compiler, VkShaderStageFlagBits stage,
    std::vector<ShaderResource> &resources) {
  auto image_resources = compiler.get_shader_resources().separate_images;

  for (auto &resource : image_resources) {
    ShaderResource shader_resource{};
    shader_resource.type = ShaderResourceType::Image;
    shader_resource.stages = stage;
    shader_resource.name = resource.name;

    read_resource_array_size(compiler, resource, shader_resource);
    read_resource_decoration<spv::DecorationDescriptorSet>(compiler, resource,
                                                           shader_resource);
    read_resource_decoration<spv::DecorationBinding>(compiler, resource,
                                                     shader_resource);

    resources.push_back(shader_resource);
  }
}

template <>
void read_shader_resource<ShaderResourceType::ImageSampler>(
    const spirv_cross::Compiler &compiler, VkShaderStageFlagBits stage,
    std::vector<ShaderResource> &resources) {
  auto image_resources = compiler.get_shader_resources().sampled_images;

  for (auto &resource : image_resources) {
    ShaderResource shader_resource{};
    shader_resource.type = ShaderResourceType::ImageSampler;
    shader_resource.stages = stage;
    shader_resource.name = resource.name;

    read_resource_array_size(compiler, resource, shader_resource);
    read_resource_decoration<spv::DecorationDescriptorSet>(compiler, resource,
                                                           shader_resource);
    read_resource_decoration<spv::DecorationBinding>(compiler, resource,
                                                     shader_resource);

    resources.push_back(shader_resource);
  }
}

template <>
void read_shader_resource<ShaderResourceType::ImageStorage>(
    const spirv_cross::Compiler &compiler, VkShaderStageFlagBits stage,
    std::vector<ShaderResource> &resources) {
  auto storage_resources = compiler.get_shader_resources().storage_images;

  for (auto &resource : storage_resources) {
    ShaderResource shader_resource{};
    shader_resource.type = ShaderResourceType::ImageStorage;
    shader_resource.stages = stage;
    shader_resource.name = resource.name;

    read_resource_array_size(compiler, resource, shader_resource);
    read_resource_decoration<spv::DecorationNonReadable>(compiler, resource,
                                                         shader_resource);
    read_resource_decoration<spv::DecorationNonWritable>(compiler, resource,
                                                         shader_resource);
    read_resource_decoration<spv::DecorationDescriptorSet>(compiler, resource,
                                                           shader_resource);
    read_resource_decoration<spv::DecorationBinding>(compiler, resource,
                                                     shader_resource);

    resources.push_back(shader_resource);
  }
}

template <>
void read_shader_resource<ShaderResourceType::Sampler>(
    const spirv_cross::Compiler &compiler, VkShaderStageFlagBits stage,
    std::vector<ShaderResource> &resources) {
  auto sampler_resources = compiler.get_shader_resources().separate_samplers;

  for (auto &resource : sampler_resources) {
    ShaderResource shader_resource{};
    shader_resource.type = ShaderResourceType::Sampler;
    shader_resource.stages = stage;
    shader_resource.name = resource.name;

    read_resource_array_size(compiler, resource, shader_resource);
    read_resource_decoration<spv::DecorationDescriptorSet>(compiler, resource,
                                                           shader_resource);
    read_resource_decoration<spv::DecorationBinding>(compiler, resource,
                                                     shader_resource);

    resources.push_back(shader_resource);
  }
}

template <>
void read_shader_resource<ShaderResourceType::BufferUniform>(
    const spirv_cross::Compiler &compiler, VkShaderStageFlagBits stage,
    std::vector<ShaderResource> &resources) {
  auto uniform_resources = compiler.get_shader_resources().uniform_buffers;

  for (auto &resource : uniform_resources) {
    ShaderResource shader_resource{};
    shader_resource.type = ShaderResourceType::BufferUniform;
    shader_resource.stages = stage;
    shader_resource.name = resource.name;

    read_resource_size(compiler, resource, shader_resource);
    read_resource_array_size(compiler, resource, shader_resource);
    read_resource_decoration<spv::DecorationDescriptorSet>(compiler, resource,
                                                           shader_resource);
    read_resource_decoration<spv::DecorationBinding>(compiler, resource,
                                                     shader_resource);

    resources.push_back(shader_resource);
  }
}

template <>
void read_shader_resource<ShaderResourceType::BufferStorage>(
    const spirv_cross::Compiler &compiler, VkShaderStageFlagBits stage,
    std::vector<ShaderResource> &resources) {
  auto storage_resources = compiler.get_shader_resources().storage_buffers;

  for (auto &resource : storage_resources) {
    ShaderResource shader_resource;
    shader_resource.type = ShaderResourceType::BufferStorage;
    shader_resource.stages = stage;
    shader_resource.name = resource.name;

    read_resource_size(compiler, resource, shader_resource);
    read_resource_array_size(compiler, resource, shader_resource);
    read_resource_decoration<spv::DecorationNonReadable>(compiler, resource,
                                                         shader_resource);
    read_resource_decoration<spv::DecorationNonWritable>(compiler, resource,
                                                         shader_resource);
    read_resource_decoration<spv::DecorationDescriptorSet>(compiler, resource,
                                                           shader_resource);
    read_resource_decoration<spv::DecorationBinding>(compiler, resource,
                                                     shader_resource);

    resources.push_back(shader_resource);
  }
}

bool SPIRVReflection::reflect_shader_resources(
    VkShaderStageFlagBits stage, const std::vector<uint32_t> &spirv,
    std::vector<ShaderResource> &resources) {
  spirv_cross::CompilerGLSL compiler{spirv};
  auto opts = compiler.get_common_options();
  opts.enable_420pack_extension = true;

  compiler.set_common_options(opts);

  parse_shader_resources(compiler, stage, resources);
  parse_push_constants(compiler, stage, resources);
  return true;
}

void SPIRVReflection::parse_shader_resources(
    const spirv_cross::Compiler &compiler, VkShaderStageFlagBits stage,
    std::vector<ShaderResource> &resources) {
  read_shader_resource<ShaderResourceType::Input>(compiler, stage, resources);
  read_shader_resource<ShaderResourceType::InputAttachment>(compiler, stage,
                                                            resources);
  read_shader_resource<ShaderResourceType::Output>(compiler, stage, resources);
  read_shader_resource<ShaderResourceType::Image>(compiler, stage, resources);
  read_shader_resource<ShaderResourceType::ImageSampler>(compiler, stage,
                                                         resources);
  read_shader_resource<ShaderResourceType::ImageStorage>(compiler, stage,
                                                         resources);
  read_shader_resource<ShaderResourceType::Sampler>(compiler, stage, resources);
  read_shader_resource<ShaderResourceType::BufferUniform>(compiler, stage,
                                                          resources);
  read_shader_resource<ShaderResourceType::BufferStorage>(compiler, stage,
                                                          resources);
}

void SPIRVReflection::parse_push_constants(
    const spirv_cross::Compiler &compiler, VkShaderStageFlagBits stage,
    std::vector<ShaderResource> &resources) {
  auto shader_resources = compiler.get_shader_resources();

  for (auto &resource : shader_resources.push_constant_buffers) {
    const auto &spivr_type = compiler.get_type_from_variable(resource.id);

    std::uint32_t offset = std::numeric_limits<std::uint32_t>::max();

    for (auto i = 0U; i < spivr_type.member_types.size(); ++i) {
      auto mem_offset = compiler.get_member_decoration(spivr_type.self, i,
                                                       spv::DecorationOffset);
      offset = std::min(offset, mem_offset);
    }

    ShaderResource shader_resource{};
    shader_resource.type = ShaderResourceType::PushConstant;
    shader_resource.stages = stage;
    shader_resource.name = resource.name;
    shader_resource.offset = offset;

    read_resource_size(compiler, resource, shader_resource);

    shader_resource.size -= shader_resource.offset;

    resources.push_back(shader_resource);
  }
}
} // namespace mango