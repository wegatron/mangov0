#pragma once

#include <engine/functional/component/basic.h>
#include <engine/functional/component/camera.h>
#include <engine/functional/component/light.h>
#include <engine/functional/component/material.h>
#include <engine/functional/component/mesh.h>
#include <entt/entt.hpp>

namespace mango {
class World final {
public:
  World() = default;

  ~World() = default;

  void update(const float seconds);

  entt::registry &camera_manager() { return camera_manager_; }
  entt::registry &light_manager() { return light_manager_; }

  entt::registry &renderableManager() { return renderable_manager_; }
  entt::entity
  createRenderableEntity(const std::string &name,
                         const std::shared_ptr<TransformRelationship> &tr,
                         const std::shared_ptr<Material> &material,
                         const std::shared_ptr<StaticMesh> &mesh);

  entt::entity
  createCameraEntity(const std::string &name,
                     const std::shared_ptr<TransformRelationship> &tr,
                     const Camera &camera);

  entt::entity
  createLightEntity(const std::string_view &name,
                    const std::shared_ptr<TransformRelationship> &tr,
                    const Light &light);

  void setRootTr(const std::shared_ptr<TransformRelationship> &root_tr) {
    root_tr_ = root_tr;
  }

  // disable copy/move
  World(const World &) = delete;
  World(World &&) = delete;
  World &operator=(const World &) = delete;
  World &operator=(World &&) = delete;

private:
  entt::registry camera_manager_;
  entt::registry light_manager_;
  entt::registry renderable_manager_;
  std::shared_ptr<TransformRelationship>
      root_tr_; // root transform relationship node
};

} // namespace mango