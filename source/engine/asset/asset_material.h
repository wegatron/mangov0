#pragma once
#include <Eigen/Dense>
#include <engine/asset/asset_texture.h>

namespace mango {

enum class ParamType : uint32_t { CONSTANT_VALUE = 0, Texture = 1 };

class Material {
public:
    
private:
  ParamType albedo_type_;
  ParamType emissive_type_;
  ParamType metallic_roughness_occlution_type_;
  Eigen::Vector4f albedo_color_;
  Eigen::Vector4f emssive_color_;
  Eigen::Vector4f metallic_roughness_occlution_;
  std::shared_ptr<AssetTexture> albedo_texture_;
  std::shared_ptr<AssetTexture> normal_texture_;
  std::shared_ptr<AssetTexture> emissive_texture_;
  std::shared_ptr<AssetTexture> metallic_roughness_occlution_texture_;
};
} // namespace mango
