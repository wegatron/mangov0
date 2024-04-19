#pragma once

#include <engine/functional/component/component_camera.h>
#include <engine/functional/component/component_transform.h>
// #include <engine/functional/component/material.h>
#include <engine/asset/url.h>
#include <engine/functional/component/component_mesh.h>
#include <entt/entt.hpp>
#include <shaders/include/shader_structs.h>

namespace mango {
class World final {
public:
  World() = default;
  ~World() = default;

  std::string getName() const { return name_; }

  void tick(const float seconds);

  /**
   * @brief 加载场景, 挂载到root_rt上.
   * @param url 场景文件路径, 可以是mango自己的格式, 也可以是其他格式,
   * 由assimp导入
   */
  void importScene(const URL &url);

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

  void setRootTr(const std::shared_ptr<TransformRelationship> &root_tr) {
    root_tr_ = root_tr;
  }

  auto getCameras() {
    return entities_.view<std::string, std::shared_ptr<TransformRelationship>,
                          CameraComponent>();
  }

  // disable copy/move
  World(const World &) = delete;
  World(World &&) = delete;
  World &operator=(const World &) = delete;
  World &operator=(World &&) = delete;

private:
  std::string name_;
  entt::registry entities_;
  std::shared_ptr<TransformRelationship>
      root_tr_; // root transform relationship node
};

} // namespace mango