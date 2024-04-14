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

  std::string getName() const { return name_; }

  void tick(const float seconds);

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

  auto getCameras() {
    return entities_.view<std::string, std::shared_ptr<TransformRelationship>, Camera>();
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