#pragma once
#include <Eigen/Dense>
#include <engine/asset/asset_texture.h>
#include <shaders/include/shader_structs.h>

namespace mango {

enum class ParamType : uint32_t { CONSTANT_VALUE = 0, Texture = 1 };

class DescriptorSet;
class Buffer;

class Material {
public:
  UMaterial &getUMaterial() { return material_; }
  void setAlbedoTexture(const std::shared_ptr<AssetTexture> &texture) {
    albedo_texture_ = texture;
  }
  void setNormalTexture(const std::shared_ptr<AssetTexture> &texture) {
    normal_texture_ = texture;
  }
  void setEmissiveTexture(const std::shared_ptr<AssetTexture> &texture) {
    emissive_texture_ = texture;
  }
  void setMetallicRoughnessOcclutionTexture(
      const std::shared_ptr<AssetTexture> &texture) {
    metallic_roughness_occlution_texture_ = texture;
  }

  void inflate();

private:
  UMaterial material_;
  std::shared_ptr<AssetTexture> albedo_texture_;
  std::shared_ptr<AssetTexture> normal_texture_;
  std::shared_ptr<AssetTexture> emissive_texture_;
  std::shared_ptr<AssetTexture> metallic_roughness_occlution_texture_;

  std::shared_ptr<DescriptorSet> descriptor_set_;
  std::shared_ptr<Buffer> material_buffer_; // uniform buffer
  uint32_t offset_{0};
};
} // namespace mango
