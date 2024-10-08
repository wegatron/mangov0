#include <engine/asset/assimp_importer.h>
#include <engine/functional/global/engine_context.h>
#include <engine/functional/world/world.h>
#include <engine/utils/base/macro.h>
#include <engine/utils/event/event_system.h>
#include <engine/utils/vk/vk_driver.h>
#include <queue>

// entt reference: https://skypjack.github.io/entt/md_docs_md_entity.html
// https://github.com/skypjack/entt/wiki/Crash-Course:-core-functionalities#introduction
namespace mango {

// entt::entity
// World::createRenderableEntity(const std::string &name,
//                               const std::shared_ptr<TransformRelationship>
//                               &tr, const std::shared_ptr<Material> &material,
//                               const std::shared_ptr<StaticMesh> &mesh) {
//   entt::entity entity = entities_.create();
//   entities_.emplace<std::string>(entity, name); // name
//   entities_.emplace<std::shared_ptr<TransformRelationship>>(
//       entity, tr); // node transform index
//   entities_.emplace<std::shared_ptr<Material>>(entity,
//                                                material); // material index
//   entities_.emplace<std::shared_ptr<StaticMesh>>(entity,
//                                                  mesh); // mesh index
//   return entity;
// }

// entt::entity
// World::createCameraEntity(const std::string &name,
//                           const std::shared_ptr<TransformRelationship> &tr,
//                           const CameraComponent &camera) {
//   entt::entity entity = entities_.create();
//   entities_.emplace<std::string>(entity, name); // name
//   entities_.emplace<std::shared_ptr<TransformRelationship>>(
//       entity,
//       tr);                                            // node transform index
//   entities_.emplace<CameraComponent>(entity, camera); // camera index
//   return entity;
// }

// entt::entity
// World::createLightEntity(const std::string_view &name,
//                          const std::shared_ptr<TransformRelationship> &tr,
//                          const Light &light) {
//   entt::entity entity = entities_.create();
//   entities_.emplace<std::string>(entity, name);
//   entities_.emplace<std::shared_ptr<TransformRelationship>>(
//       entity,
//       tr); // node transform index
//   entities_.emplace<Light>(entity, light);
//   return entity;
// }

World::World() {
  g_engine.getEventSystem()->addListener(
      EEventType::ImportScene, [this](const EventPointer &event) {
        auto e = std::static_pointer_cast<ImportSceneEvent>(event);
        importScene(e->file_path);
      });
  // default root tr
  root_tr_ = std::make_shared<TransformRelationship>();

  // default camera
  auto camera = CameraComponent();
  camera.setFovy(M_PI / 6.0f);
  camera.setClipPlanes(-0.1f, -1000.0f);
  default_camera_ = createEntity("default##camera");
  addComponent(default_camera_, camera);
}

void World::loadedMesh2World() {
  auto prev_frame_index = g_engine.getDriver()->getPrevFrameIndex();
  auto &scene_data_list = imported_scene_datas_[prev_frame_index];
  if (scene_data_list.empty())
    return;
  for (auto &scene_data : scene_data_list) {
    scene_data.scene_root_tr->parent = root_tr_;
    scene_data.scene_root_tr->sibling = root_tr_->child;
    root_tr_->child = scene_data.scene_root_tr;
    for (auto &mesh_entity_dat : scene_data.mesh_entity_datas) {
      auto entity = createEntity(mesh_entity_dat.name);
      addComponent(entity, mesh_entity_dat.tr);
      addComponent(entity, mesh_entity_dat.mesh);
      addComponent(entity, mesh_entity_dat.material);
    }

    //// for lighting
    for (auto &light_entity_dat : scene_data.light_entity_datas) {
      auto entity = createEntity(light_entity_dat.name);
      addComponent(entity, light_entity_dat.tr);
      auto light_type = light_entity_dat.light_type;
      auto light_index = light_entity_dat.light_index;
      assert(light_type < LightType::LIGHT_TYPE_NUM);
      if (lighting_.light_num[light_index] < MAX_LIGHT_NUM[light_type]) {
        light_index += lighting_.light_num[light_type];
        addComponent(entity, light_index);
        lighting_dirty_ = true; // need to update light ubo
      } else {
        LOGW("light num of light type {} exceed max limit", light_type);
      }
    }

    // copy lighting data to ubo
    void *ptr[] = {lighting_.directional_lights +
                       sizeof(UDirectionalLight) *
                           lighting_.light_num[LightType::DIRECTIONAL],
                   lighting_.point_lights +
                       sizeof(UPointLight) *
                           lighting_.light_num[LightType::POINT]};
    size_t cp_size[] = {sizeof(UDirectionalLight), sizeof(UPointLight)};
    static_assert(sizeof(ptr) / sizeof(void *) == LightType::LIGHT_TYPE_NUM,
                  "light type num not match");
    static_assert(sizeof(cp_size) / sizeof(size_t) == LightType::LIGHT_TYPE_NUM,
                  "light type num not match");
    for (auto light_type = 0; light_type < LightType::LIGHT_TYPE_NUM;
         ++light_type) {
      auto cp_num =
          std::min(static_cast<ushort>(MAX_LIGHT_NUM[light_type] -
                                       lighting_.light_num[light_type]),
                   scene_data.lighting.light_num[light_type]);
      memcpy(ptr[light_type], scene_data.lighting.directional_lights,
             cp_size[light_type] * cp_num);
      lighting_.light_num[light_type] += cp_num;
      lighting_dirty_ = true;
    }
  }
  scene_data_list.clear();
  focus_camera2world_ = true;
}

void World::updateTransform() {
  auto &scene_aabb = root_tr_->aabb;
  scene_aabb.setEmpty();
  //// update rt
  std::queue<std::shared_ptr<TransformRelationship>> q;
  if (root_tr_->child != nullptr)
    q.emplace(root_tr_->child);
  while (!q.empty()) {
    auto node = q.front();
    q.pop();
    // visit node
    auto parent = node->parent;
    node->gtransform = parent->gtransform * node->ltransform;
    scene_aabb.extend(
        node->aabb.transformed(Eigen::Affine3f(node->gtransform)));
    // TODO update aabb for dynamic mesh

    // visit sibling
    if (node->sibling != nullptr) {
      q.emplace(node->sibling);
    }
    // visit child
    if (node->child != nullptr) {
      q.emplace(node->child);
    }
  }
}

void World::updateCamera() {
  // auto view = getCameras();
  // for (auto entity : view) {
  //   auto &camera = view.get<CameraComponent>(entity);
  //   auto &tr = view.get<TransformComponent>(entity);
  //   camera.setViewMatrix(tr->gtransform.inverse().matrix());
  // }

  if (focus_camera2world_) {
    auto &camera_comp = entities_.get<CameraComponent>(default_camera_);
    float dis = root_tr_->aabb.sizes().norm() * 2;
    Eigen::Vector3f c = root_tr_->aabb.center();
    Eigen::Vector3f eye = c + Eigen::Vector3f(0, 0, 1) * dis;
    camera_comp.setLookAt(eye, Eigen::Vector3f(0, 1, 0), c);
    focus_camera2world_ = false;
  }
}

void World::tick(const float seconds) {
  // append new imported scene to root
  loadedMesh2World();
  updateTransform();
  updateCamera();
}

void World::importScene(const std::string &url) {
  bool suc = AssimpImporter::import(url, this);
  if (!suc) {
    LOGE("import scene failed: {}", url.c_str());
  }
}

void World::saveAsWorld(const URL &url) {}

void World::saveWorld() {}

} // namespace mango