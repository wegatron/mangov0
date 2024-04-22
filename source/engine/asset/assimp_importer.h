#pragma once

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <engine/asset/url.h>
// #include <engine/functional/component/component_transform.h>

namespace mango {
class World;
class StaticMeshComponent;
class MaterialComponent;
class TransformRelationship;
class CommandBuffer;
class CameraComponent;

class AssimpImporter final {
public:
  AssimpImporter() = default;
  static bool import(const URL &url, World *world);

private:
  std::shared_ptr<TransformRelationship>
  processNode(const std::shared_ptr<TransformRelationship> &parent,
              aiNode *node, const aiScene *a_scene, World &scene,
              std::vector<std::shared_ptr<StaticMeshComponent>> &meshes,
              std::vector<std::shared_ptr<MaterialComponent>> &materials);

  std::vector<std::shared_ptr<StaticMeshComponent>>
  processMeshs(const aiScene *a_scene,
               const std::shared_ptr<CommandBuffer> &cmd_buf);

  std::vector<std::shared_ptr<MaterialComponent>>
  processMaterials(const aiScene *a_scene, const std::string &dir,
                   const std::shared_ptr<CommandBuffer> &cmd_buf);

  std::vector<CameraComponent> processCameras(const aiScene *a_scene);

  //   Lights processLight(const aiScene *a_scene);
  // void loadAndSet(const std::string &dir, const aiScene *a_scene, aiMaterial
  // *a_mat,
  //                 const std::shared_ptr<CommandBuffer> &cmd_buf,
  //                 aiTextureType ttype, const char *pKey, unsigned int vtype,
  //                 unsigned int idx, const char *shader_texture_name,
  //                 const char *shader_color_name,
  //                 std::shared_ptr<PbrMaterial> &mat);
};
} // namespace mango