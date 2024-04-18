#pragma once

#include <engine/functional/component/basic.h>
#include <engine/functional/component/camera.h>
// #include <engine/functional/component/material.h>
#include <engine/functional/component/mesh.h>
#include <entt/entt.hpp>
#include <shaders/include/shader_structs.h>

namespace mango {
class World final {
public:
  World() = default;
  ~World() = default;

  std::string getName() const { return name_; }

  void tick(const float seconds);

  entt::entity createEntity(const std::string &name) {
    return entities_.create();
  }

  void addComponent(entt::entity entity,
                    const std::shared_ptr<TransformRelationship> &tr) {
    entities_.emplace<std::shared_ptr<TransformRelationship>>(entity, tr);
  }

  void addComponent(entt::entity entity, const Camera &camera) {
    entities_.emplace<Camera>(entity, camera);
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
    return entities_
        .view<std::string, std::shared_ptr<TransformRelationship>, Camera>();
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