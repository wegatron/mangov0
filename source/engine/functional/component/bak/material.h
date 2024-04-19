#pragma once
#include <algorithm>
#include <glm/glm.hpp>
#include <list>
#include <map>
#include <stdexcept>
#include <string_view>
#include <vulkan/vulkan.h>

#include <engine/utils/vk/buffer.h>
#include <engine/utils/vk/descriptor_set.h>
#include <engine/utils/vk/pipeline_state.h>
#include <engine/utils/vk/shader_module.h>
#include <engine/utils/vk/vk_constants.h>
#include <engine/utils/vk/vk_driver.h>

namespace mango {

// using uint64_t to define a material type code, the higher 16 bit for Basic
// Material type, and the lower 16 bits for variant input.
#define PBR_MATERIAL 1u << 16

constexpr char const *BASE_COLOR_NAME = "pbr_mat.base_color";
constexpr char const *METALLIC_NAME = "pbr_mat.metallic";
constexpr char const *ROUGHNESS_NAME = "pbr_mat.roughness";
constexpr char const *SPECULAR_NAME = "pbr_mat.specular";

constexpr char const *BASE_COLOR_TEXTURE_NAME = "base_color_tex";
constexpr char const *METALLIC_TEXTURE_NAME = "metallic_tex";
constexpr char const *ROUGHNESS_TEXTURE_NAME = "roughness_tex";
constexpr char const *METALLIC_ROUGHNESS_TEXTURE_NAME =
    "metallic_roughness_tex";
constexpr char const *SPECULAR_TEXTURE_NAME = "specular_tex";
constexpr char const *NORMAL_TEXTURE_NAME = "normal_map";

enum PbrTextureParamIndex {
  BASE_COLOR_TEXTURE_INDEX = 0,
  METALLIC_TEXTURE_INDEX = 1,
  ROUGHNESS_TEXTURE_INDEX = 2,
  METALLIC_ROUGHNESS_TEXTURE_INDEX = 3,
  SPECULAR_TEXTURE_INDEX = 4,
  NORMAL_TEXTURE_INDEX = 5,
  MAT_TEXTURE_NUM_COUNT
};

class GraphicsPipeline;
class ImageView;
class RenderPass;
class Sampler;

struct MaterialUboParam {
  uint32_t stride{0}; // for array element in uniform buffer
  const uint32_t ub_offset;
  const std::type_info &tinfo;
  std::string name;
};

struct MaterialUboInfo {
  uint32_t set;
  uint32_t binding;
  uint32_t size;
  bool dirty{false};
  std::vector<std::byte> data;
  std::vector<MaterialUboParam> params;
};

struct MaterialTextureParam {
  uint32_t set;
  uint32_t binding;
  uint32_t index; // for array texture
  const char *name;
  const char *def; // add definition to shader
  std::shared_ptr<mango::ImageView> img_view;
  std::shared_ptr<mango::Sampler> sampler;
  bool dirty;
};

struct MatParamsSet {
  uint32_t mat_type_id{0};
  std::unique_ptr<Buffer> ubo;
  std::shared_ptr<DescriptorSet> desc_set;
};

class Material;

/**
 * \brief MatGpuResourcePool is a gpu resource pool for material.
 * It manages GraphicsPipeline, MatParamsSet(uniform buffer + DescriptorSet)
 *
 * the gc function should be called onece per frame
 */
class MatGpuResourcePool {
public:
  MatGpuResourcePool(VkFormat color_format, VkFormat ds_format);

  void gc();

  std::shared_ptr<GraphicsPipeline>
  requestGraphicsPipeline(const std::shared_ptr<Material> &mat);

  std::shared_ptr<DescriptorSet>
  requestMatDescriptorSet(const std::shared_ptr<Material> &mat);

private:
  std::shared_ptr<RenderPass> default_render_pass_;
  std::map<uint32_t, std::shared_ptr<GraphicsPipeline>> mat_pipelines_;
  std::unique_ptr<DescriptorPool> desc_pool_;
  std::list<std::shared_ptr<MatParamsSet>> used_mat_params_set_;
  std::list<std::shared_ptr<MatParamsSet>> free_mat_params_set_;
};

/**
 * \brief Material defines the texture of the rendered object,
 * specifying the shaders in the rendering pipeline, as well as its related
 * rendering states, inputs, and so on
 *
 * One can construct material input parameters buffer/push const using Material
 * interface.
 */
class Material {
public:
  Material() = default;

  virtual ~Material() = default;

  template <typename T>
  void setUboParamValue(const std::string &name, const T value,
                        uint32_t index = 0) {
    for (auto &param : ubo_info_.params) {
      if (param.name == name) {
        // do the job
        assert(param.tinfo == typeid(T));
        uint32_t param_offset = index * param.stride + param.ub_offset;
        if ((index != 0 && param.stride == 0) ||
            param_offset + sizeof(T) >= ubo_info_.size)
          throw std::runtime_error("invalid ubo param index");
        memcpy(ubo_info_.data.data() + param_offset, &value, sizeof(T));
        ubo_info_.dirty = true;
        return;
      }
    }
    throw std::runtime_error("invalid ubo param name or type");
  }

  void setTexture(const std::string &name,
                  const std::shared_ptr<ImageView> &img_view,
                  uint32_t index = 0) {
    auto itr = std::find_if(texture_params_.begin(), texture_params_.end(),
                            [&name, &index](const MaterialTextureParam &param) {
                              return param.name == name && param.index == index;
                            });
    assert(itr != texture_params_.end());
    itr->img_view = img_view;
    itr->dirty = true;
  }

  std::vector<MaterialTextureParam> &textureParams() { return texture_params_; }

  void updateParams();

  /**
   * \brief update the information(vs,fs, multisample, subpass index) to
   * pipeline state
   */
  virtual void setPipelineState(GPipelineState &pipeline_state) = 0;

  virtual void compile() = 0;

  uint32_t materialTypeId() const { return material_type_id_; }

protected:
  virtual std::shared_ptr<MatParamsSet>
  createMatParamsSet(const std::shared_ptr<VkDriver> &driver,
                     DescriptorPool &desc_pool) = 0;

  std::shared_ptr<ShaderModule> vs_;
  std::shared_ptr<ShaderModule> gs_;
  std::shared_ptr<ShaderModule> fs_;

  // std::shared_ptr<PipelineState> pipeline_state_;
  // std::vector<ShaderResource> shader_resources_;

  MaterialUboInfo ubo_info_; //!< params info(specification、cpu data) of
                             //!< material store in uniform buffer

  std::vector<MaterialTextureParam>
      texture_params_; //!< texture params of material

  std::shared_ptr<MatParamsSet> mat_param_set_;
  std::unique_ptr<DescriptorSetLayout> desc_set_layout_;

  uint32_t material_type_id_{
      0}; //!< using uint64_t to define a material type code, the higher 16 bit
          //!< for Basic Material type, and the lower 16 bits for variant input.

  friend class MatGpuResourcePool;
  // uint32_t variance_; // material variance bit flags, check by value
};
} // namespace mango