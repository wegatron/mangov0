#include <engine/asset/asset_manager.h>
#include <engine/asset/texture.h>
#include <engine/functional/global/engine_context.h>
#include <engine/platform/file_system.h>

namespace mango {

void AssetManager::init() {
  asset_type_exts_ = {
      {EAssetType::TEXTURE2D, "tex"},  {EAssetType::TEXTURECUBE, "texc"},
      {EAssetType::TEXTURE2D, "png"},  {EAssetType::TEXTURE2D, "jpg"},
      {EAssetType::MATERIAL, "mat"},   {EAssetType::SKELETON, "skl"},
      {EAssetType::STATICMESH, "sm"},  {EAssetType::SKELETALMESH, "skm"},
      {EAssetType::ANIMATION, "anim"}, {EAssetType::WORLD, "world"}};

  asset_archive_types_ = {{EAssetType::TEXTURE2D, EArchiveType::BINARY},
                          {EAssetType::TEXTURECUBE, EArchiveType::BINARY},
                          {EAssetType::MATERIAL, EArchiveType::JSON},
                          {EAssetType::SKELETON, EArchiveType::BINARY},
                          {EAssetType::STATICMESH, EArchiveType::BINARY},
                          {EAssetType::SKELETALMESH, EArchiveType::BINARY},
                          {EAssetType::ANIMATION, EArchiveType::BINARY},
                          {EAssetType::WORLD, EArchiveType::JSON}};

  for (const auto &iter : asset_type_exts_) {
    ext_asset_types_[iter.second] = iter.first;
  }
}

EAssetType AssetManager::getAssetType(const URL &url) {
  std::string extension = g_engine.getFileSystem()->extension(url.str());
  if (ext_asset_types_.find(extension) != ext_asset_types_.end()) {
    return ext_asset_types_[extension];
  }
  return EAssetType::INVALID;
}

std::shared_ptr<Asset> AssetManager::deserializeAsset(const URL &url) {

  if (assets_.find(url) != assets_.end()) {
    return assets_[url];
  }

  EAssetType asset_type = getAssetType(url);
  if (asset_type == EAssetType::INVALID) {
    throw std::runtime_error("unsupported asset type");
  }

  std::shared_ptr<Asset> asset = nullptr;
  switch (asset_type) {
  case EAssetType::TEXTURE2D:
    asset = std::make_shared<AssetTexture>();
    break;
  default:
    throw std::runtime_error("unsupported asset type");
  }

  asset->load(url);
  assets_[url] = asset;
  return asset;
}
} // namespace mango