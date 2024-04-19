#pragma once
#include <engine/asset/asset.h>
#include <engine/asset/url.h>
#include <map>
#include <memory>
#include <string>

namespace mango {
enum class EArchiveType { JSON, BINARY };

/**
 * @brief AssetManager 只负责 保存和加载(序列化/反序列化)mango自己的资源,
 * 以及图片, 对于场景资源文件, 由world调用assimp_importer来导入
 */
class AssetManager final {
public:
  void init();
  void destroy();

  bool import3d(const URL &url);

  EAssetType getAssetType(const URL &url);

  template <typename AssetClass>
  std::shared_ptr<AssetClass> loadAsset(const URL &url) {
    std::shared_ptr<Asset> asset = deserializeAsset(url);
    return std::dynamic_pointer_cast<AssetClass>(asset);
  }

  void serializeAsset(std::shared_ptr<Asset> asset,
                      const std::string &file_path = "");

private:
  std::shared_ptr<Asset> deserializeAsset(const URL &file_path);
  std::string getAssetName(const std::string &asset_name, EAssetType asset_type,
                           int asset_index = 0,
                           const std::string &basename = "");

  std::map<URL, std::shared_ptr<Asset>> assets_;
  std::map<EAssetType, std::string> asset_type_exts_;
  std::map<EAssetType, EArchiveType> asset_archive_types_;
  std::map<std::string, EAssetType> ext_asset_types_;
};

} // namespace mango