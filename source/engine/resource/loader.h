#pragma once

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <engine/functional/component/light.h>
#include <engine/functional/global/app_context.h>
#include <engine/functional/world/world.h>
#include <engine/utils/vk/vk_driver.h>

namespace mango {

class CommandBuffer;
class StagePool;
class GPUAssetCache;
class Camera;

// todo Static Mesh, Material TransformRelationship memory management
class AssimpLoader final {
public:
  AssimpLoader() = default;

  void loadScene(const std::string &path, World &scene,
                 const std::shared_ptr<CommandBuffer> &cmd_buf);

private:
  std::shared_ptr<TransformRelationship>
  processNode(const std::shared_ptr<TransformRelationship> &parent,
              aiNode *node, const aiScene *a_scene, World &scene,
              std::vector<std::shared_ptr<StaticMesh>> &meshes,
              std::vector<std::shared_ptr<Material>> &materials);

  std::vector<std::shared_ptr<StaticMesh>>
  processMeshs(const aiScene *a_scene,
               const std::shared_ptr<CommandBuffer> &cmd_buf);

  std::vector<std::shared_ptr<Material>>
  processMaterials(const aiScene *a_scene, const std::string &dir,
                   const std::shared_ptr<CommandBuffer> &cmd_buf);

  std::vector<Camera> processCameras(const aiScene *a_scene);

  Lights processLight(const aiScene *a_scene);
  // void loadAndSet(const std::string &dir, const aiScene *a_scene, aiMaterial
  // *a_mat,
  //                 const std::shared_ptr<CommandBuffer> &cmd_buf,
  //                 aiTextureType ttype, const char *pKey, unsigned int vtype,
  //                 unsigned int idx, const char *shader_texture_name,
  //                 const char *shader_color_name,
  //                 std::shared_ptr<PbrMaterial> &mat);
};
} // namespace mango