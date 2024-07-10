#pragma once

#include <entt/entt.hpp>
#include <mutex>

#include <engine/functional/component/component_camera.h>
#include <engine/functional/component/component_transform.h>
#include <engine/functional/component/components.h>
#include <engine/functional/global/engine_context.h>
#include <engine/asset/asset_material.h>
#include <engine/asset/url.h>
#include <shaders/include/shader_structs.h>
#include <engine/utils/vk/vk_constants.h>
#include <engine/utils/vk/vk_driver.h>



namespace mango {

struct MeshEntityData {
  std::string name;
  std::shared_ptr<StaticMesh> mesh;
  std::shared_ptr<Material> material;
  std::shared_ptr<TransformRelationship> tr;
};

struct LightEntityData {
  std::string name;
  std::shared_ptr<TransformRelationship> tr;
  uint16_t light_type;
  uint16_t light_index;
};

struct ImportedSceneData {
  std::shared_ptr<TransformRelationship> scene_root_tr;
  std::vector<MeshEntityData> mesh_entity_datas;
  std::vector<LightEntityData> light_entity_datas;
  ULighting lighting;
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
    auto ret = entities_.create();
    entities_.emplace<std::string>(ret, name);
    return ret;
  }

  void removeEntity(entt::entity entity) { entities_.destroy(entity); }

  template<typename T>
  void addComponent(entt::entity entity, const T &comp) {
    entities_.emplace<T>(entity, comp);
  }

  auto getCameras() {
    return entities_.view<std::string, std::shared_ptr<TransformRelationship>,
                          CameraComponent>();
  }

  auto &getDefaultCameraComp()
  {
    return entities_.get<CameraComponent>(default_camera_);
  }

  auto getStaticMeshes() {
    return entities_.view<std::string, TransformComponent, StaticMeshComponent, MaterialComponent>();
  }

  void enqueue(const std::shared_ptr<TransformRelationship> &tr,
               std::vector<MeshEntityData> &&mesh_entity_datas,
               std::vector<LightEntityData> &&light_entity_datas,
               const ULighting &lighting) {
    auto driver = g_engine.getDriver();
    auto &dat = imported_scene_datas_[driver->getCurFrameIndex()].emplace_back();
    dat.scene_root_tr = tr;
    dat.mesh_entity_datas = std::move(mesh_entity_datas);
    dat.light_entity_datas = std::move(light_entity_datas);
    dat.lighting = lighting;
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
  
  // light ubo data
  ULighting lighting_;
  bool lighting_dirty_{true};
  
  bool focus_camera2world_{false};
};

} // namespace mango