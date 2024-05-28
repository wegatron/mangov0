#pragma once

#include <entt/entt.hpp>
#include <mutex>

#include <engine/functional/component/component_camera.h>
#include <engine/functional/component/component_transform.h>
#include <engine/functional/component/component_mesh.h>
#include <engine/functional/global/engine_context.h>

// #include <engine/functional/component/material.h>
#include <engine/asset/url.h>
#include <shaders/include/shader_structs.h>


namespace mango {

struct MeshEntityData {
  std::string name;
  std::shared_ptr<StaticMesh> mesh;
  std::shared_ptr<TransformRelationship> tr;
};

struct ImportedSceneData {
  std::shared_ptr<TransformRelationship> scene_root_tr;
  std::vector<MeshEntityData> mesh_entity_datas;
};

class World final {
public:
  World();

  ~World() = default;

  std::string getName() const { return name_; }

  void tick(const float seconds);

  /**
   * @brief 加载场景, 挂载到root_rt上.
   * @param url 场景文件路径, 可以是mango自己的格式, 也可以是其他格式,
   * 由assimp导入
   */
  void importScene(const std::string &url);

  void saveAsWorld(const URL &url);

  void saveWorld();

  entt::entity createEntity(const std::string &name) {
    return entities_.create();
  }

  void removeEntity(entt::entity entity) { entities_.destroy(entity); }

  void addComponent(entt::entity entity,
                    const std::shared_ptr<TransformRelationship> &tr) {
    entities_.emplace<std::shared_ptr<TransformRelationship>>(entity, tr);
  }

  void addComponent(entt::entity entity, const CameraComponent &camera) {
    entities_.emplace<CameraComponent>(entity, camera);
  }

  void addComponent(entt::entity entity, const PointLight &light) {
    entities_.emplace<PointLight>(entity, light);
  }

  void addComponent(entt::entity entity, const DirectionalLight &light) {
    entities_.emplace<DirectionalLight>(entity, light);
  }

  void addComponent(entt::entity entity, const StaticMeshComponent &mesh) {
    entities_.emplace<StaticMeshComponent>(entity, mesh);
  }

  auto getCameras() {
    return entities_.view<std::string, std::shared_ptr<TransformRelationship>,
                          CameraComponent>();
  }

  auto getStaticMeshes() {
    return entities_.view<std::string, std::shared_ptr<TransformRelationship>,
                          StaticMeshComponent>();
  }

  void enqueue(const std::shared_ptr<TransformRelationship> &tr,
               std::vector<MeshEntityData> &&mesh_entity_datas) {
    auto driver = g_engine.getDriver();
    auto &dat = imported_scene_datas_[driver->getCurFrameIndex()].emplace_back();
    dat.scene_root_tr = tr;
    dat.mesh_entity_datas = std::move(mesh_entity_datas);
  }

  void focusCamera2World() { focus_camera2world_ = true; }

  // disable copy/move
  World(const World &) = delete;
  World(World &&) = delete;
  World &operator=(const World &) = delete;
  World &operator=(World &&) = delete;

private:
  /**
   * @brief 将预加载的mesh数据加载到世界中
   */
  void loadedMesh2World();

  void updateTransform();

  void updateCamera();

  std::string name_;
  entt::registry entities_;
  std::shared_ptr<TransformRelationship>
      root_tr_; // root transform relationship node
  std::vector<ImportedSceneData> imported_scene_datas_[MAX_FRAMES_IN_FLIGHT];
  entt::entity default_camera_;
  bool focus_camera2world_{false};
};

} // namespace mango