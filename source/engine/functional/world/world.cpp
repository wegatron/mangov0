#include <engine/functional/world/world.h>
#include <queue>

// entt reference: https://skypjack.github.io/entt/md_docs_md_entity.html
// https://github.com/skypjack/entt/wiki/Crash-Course:-core-functionalities#introduction
namespace mango {

entt::entity
World::createRenderableEntity(const std::string &name,
                              const std::shared_ptr<TransformRelationship> &tr,
                              const std::shared_ptr<Material> &material,
                              const std::shared_ptr<StaticMesh> &mesh) {
  entt::entity entity = renderable_manager_.create();
  renderable_manager_.emplace<std::string>(entity, name); // name
  renderable_manager_.emplace<std::shared_ptr<TransformRelationship>>(
      entity, tr); // node transform index
  renderable_manager_.emplace<std::shared_ptr<Material>>(
      entity, material); // material index
  renderable_manager_.emplace<std::shared_ptr<StaticMesh>>(entity,
                                                           mesh); // mesh index
  return entity;
}

entt::entity
World::createCameraEntity(const std::string &name,
                          const std::shared_ptr<TransformRelationship> &tr,
                          const Camera &camera) {
  entt::entity entity = camera_manager_.create();
  camera_manager_.emplace<std::string>(entity, name); // name
  camera_manager_.emplace<std::shared_ptr<TransformRelationship>>(
      entity,
      tr);                                         // node transform index
  camera_manager_.emplace<Camera>(entity, camera); // camera index
  return entity;
}

entt::entity
World::createLightEntity(const std::string_view &name,
                         const std::shared_ptr<TransformRelationship> &tr,
                         const Light &light) {
  entt::entity entity = light_manager_.create();
  light_manager_.emplace<std::string>(entity, name);
  light_manager_.emplace<std::shared_ptr<TransformRelationship>>(
      entity,
      tr); // node transform index
  light_manager_.emplace<Light>(entity, light);
  return entity;
}

void World::update(const float seconds) {
  auto &scene_aabb = root_tr_->aabb;
  scene_aabb.setEmpty();
  //// update rt
  std::queue<std::shared_ptr<TransformRelationship>> q;
  // add root->node's children
  auto rch = root_tr_->child;
  if (rch != nullptr) {
    rch->gtransform = rch->ltransform;
    scene_aabb.extend(rch->aabb.transformed(Eigen::Affine3f(rch->gtransform)));
  }
  while (rch != nullptr) // add child nodes
  {
    q.emplace(rch);
    rch = rch->sibling;
  }
  while (!q.empty()) {
    auto node = q.front();
    q.pop();
    node->gtransform = node->parent->gtransform * node->ltransform;
    scene_aabb.extend(
        node->aabb.transformed(Eigen::Affine3f(node->gtransform)));
    // add child nodes
    auto ch = node->child;
    while (ch != nullptr) {
      q.emplace(ch);
      ch = ch->sibling;
    }
  }
}

} // namespace mango