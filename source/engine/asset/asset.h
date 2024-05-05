#pragma once

#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/vector.hpp>
#include <engine/asset/url.h>

namespace mango {
enum class EAssetType {
  INVALID,
  TEXTURE2D,
  TEXTURECUBE,
  MATERIAL,
  SKELETON,
  STATICMESH,
  SKELETALMESH,
  ANIMATION,
  WORLD,
  SCENE,
};

class Asset {
public:
  Asset() = default;
  virtual ~Asset() = default;
  void setURL(const URL &url);

  const URL &getURL() { return url_; }

  EAssetType getAssetType() { return asset_type_; }

  virtual void load(const URL &url) = 0;

protected:
  URL url_;
  EAssetType asset_type_;

private:
  friend class cereal::access;
  template <class Archive> void serialize(Archive &ar) {}
};
} // namespace mango