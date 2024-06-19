#include <engine/asset/assimp_importer.h>

#include <queue>
#include <Eigen/Dense>
#include <engine/utils/base/macro.h>
#include <engine/utils/vk/commands.h>
#include <engine/asset/asset_mesh.h>
#include <engine/asset/asset_material.h>
#include <engine/functional/component/component_camera.h>
#include <engine/functional/component/component_transform.h>
#include <engine/functional/global/engine_context.h>
#include <engine/functional/world/world.h>


namespace mango {

std::vector<std::shared_ptr<StaticMesh>>
processMeshs(const aiScene *a_scene) {
  auto a_meshes = a_scene->mMeshes;
  std::vector<std::shared_ptr<StaticMesh>> ret_meshes(a_scene->mNumMeshes);
  for (auto i = 0; i < a_scene->mNumMeshes; ++i) {
    auto tmp_a_mesh = a_meshes[i];
    ret_meshes[i] = std::make_shared<StaticMesh>();

    // mesh data to static mesh data
    // vertices data: 3f_pos | 3f_normal | 2f_uv
    auto nv = tmp_a_mesh->mNumVertices;
    std::vector<StaticVertex> vertices(nv);
    static_assert(std::is_same<ai_real, float>::value,
                  "Type should be same while using memory copy.");
    for (auto vi = 0; vi < nv; ++vi) {
      vertices[vi].position = Eigen::Vector3f(tmp_a_mesh->mVertices[vi].x,
                                              tmp_a_mesh->mVertices[vi].y,
                                              tmp_a_mesh->mVertices[vi].z);
      vertices[vi].normal = Eigen::Vector3f(tmp_a_mesh->mNormals[vi].x,
                                            tmp_a_mesh->mNormals[vi].y,
                                            tmp_a_mesh->mNormals[vi].z);
      vertices[vi].uv =
          Eigen::Vector2f(tmp_a_mesh->mTextureCoords[0][vi].x,  // NOLINT
                          tmp_a_mesh->mTextureCoords[0][vi].y); // NOLINT
    }
    ret_meshes[i]->setVertices(std::move(vertices));

    // faces
    auto nf = tmp_a_mesh->mNumFaces;
    std::vector<uint32_t> tri_v_inds(nf * 3);
    for (auto j = 0; j < nf; ++j) {
      assert(tmp_a_mesh->mFaces[j].mNumIndices == 3);
      tri_v_inds[j * 3] = tmp_a_mesh->mFaces[j].mIndices[0];
      tri_v_inds[j * 3 + 1] = tmp_a_mesh->mFaces[j].mIndices[1];
      tri_v_inds[j * 3 + 2] = tmp_a_mesh->mFaces[j].mIndices[2];
    }
    ret_meshes[i]->setIndices(tri_v_inds);
    ret_meshes[i]->setSubMeshs({{nf*3, 0}});
    ret_meshes[i]->inflate();
  }
  return ret_meshes;
}


std::shared_ptr<AssetTexture> loadTexture(const aiTexture *a_texture,
                                          const char *texture_path) {
  auto ret_texture = std::make_shared<AssetTexture>();
  if (a_texture == nullptr) {
    ret_texture->load(texture_path);
  } else {
    int width, height, channels;
    stbi_uc *data = stbi_load_from_memory(
        reinterpret_cast<const stbi_uc *>(a_texture->pcData), a_texture->mWidth,
        &width, &height, &channels, STBI_rgb_alpha);
    if (data == nullptr) {
      throw std::runtime_error("Failed to load texture: " +
                              std::string(texture_path));
    }
    ret_texture->load(width, height, data);
    stbi_image_free(data);
  }
  return ret_texture;
}

std::vector<std::shared_ptr<Material>>
processMaterials(const aiScene *a_scene, const std::string &dir)
{
  std::vector<std::shared_ptr<Material>> ret_mats(a_scene->mNumMaterials);
  for (uint32_t i = 0; i < a_scene->mNumMaterials; ++i)
  {
    auto a_mat = a_scene->mMaterials[i];
    auto cur_mat = std::make_shared<Material>();
    ret_mats[i] = cur_mat;
    aiString texture_path;
    auto &u_material = cur_mat->getUMaterial();
    if (AI_SUCCESS ==
        a_mat->GetTexture(aiTextureType_BASE_COLOR, 0, &texture_path)) {
      u_material.albedo_type = static_cast<uint32_t>(ParamType::Texture);
      auto a_texture = a_scene->GetEmbeddedTexture(texture_path.C_Str());
      std::string texture_path_str = dir + std::string(texture_path.C_Str());
      // load texture data from memory or file file
      cur_mat->setAlbedoTexture(
          loadTexture(a_texture, texture_path_str.c_str()));
    } else {
      u_material.albedo_type = static_cast<uint32_t>(ParamType::CONSTANT_VALUE);
      aiColor3D value(0.0f, 0.0f, 0.0f);
      a_mat->Get(AI_MATKEY_BASE_COLOR, value);
      u_material.albedo_color = Eigen::Vector4f(value.r, value.g, value.b, 1.0);
    }
    if (AI_SUCCESS == a_mat->GetTexture(aiTextureType_NORMALS, 0, &texture_path))
    {      
      auto a_texture = a_scene->GetEmbeddedTexture(texture_path.C_Str());
      std::string texture_path_str = dir + std::string(texture_path.C_Str());
      cur_mat->setNormalTexture(loadTexture(a_texture, texture_path_str.c_str()));
    }
    if (AI_SUCCESS == a_mat->GetTexture(aiTextureType_EMISSIVE, 0, &texture_path))
    {
      u_material.emissive_type = static_cast<uint32_t>(ParamType::Texture);
      auto a_texture = a_scene->GetEmbeddedTexture(texture_path.C_Str());
      std::string texture_path_str = dir + std::string(texture_path.C_Str());
      cur_mat->setEmissiveTexture(loadTexture(a_texture, texture_path_str.c_str()));      
    } else {
      u_material.emissive_type = static_cast<uint32_t>(ParamType::CONSTANT_VALUE);
      aiColor3D value(0.0f, 0.0f, 0.0f);
      a_mat->Get(AI_MATKEY_COLOR_EMISSIVE, value);
      u_material.emissive_color = Eigen::Vector4f(value.r, value.g, value.b, 1.0f);
    }

    // assert metallic and roughness share the same texture
    if (AI_SUCCESS == a_mat->GetTexture(AI_MATKEY_ROUGHNESS_TEXTURE, &texture_path))
    {
      u_material.metallic_roughness_occlution_type = static_cast<uint32_t>(ParamType::Texture);
      auto a_texture = a_scene->GetEmbeddedTexture(texture_path.C_Str());
      std::string texture_path_str = dir + std::string(texture_path.C_Str());
      cur_mat->setMetallicRoughnessOcclutionTexture(loadTexture(a_texture, texture_path_str.c_str()));  
    } else {
      u_material.metallic_roughness_occlution_type = static_cast<uint32_t>(ParamType::CONSTANT_VALUE);
      float value = 0.0f;
      a_mat->Get(AI_MATKEY_METALLIC_FACTOR, value);
      u_material.metallic_roughness_occlution[0] = value;
      a_mat->Get(AI_MATKEY_ROUGHNESS_FACTOR, value);
      u_material.metallic_roughness_occlution[1] = value;
    }
  }
  return ret_mats;
}

// Lights processLights(const aiScene *a_scene) {
//   // There is bugs in assimp load light from gltf
//   // Lights lights{a_scene->mNumLights};
//   // for (auto i=0; i<a_scene->mNumLights; ++i)
//   // {
//   //   auto a_light = a_scene->mLights[i];
//   //   auto &l = lights.l[i];
//   //   l.light_type = static_cast<LightType>(a_light->mType);
//   //   l.inner_angle = a_light->mAngleInnerCone;
//   //   l.outer_angle = a_light->mAngleOuterCone;
//   //   l.intensity = a_light->mAttenuationConstant;
//   //   l.position = Eigen::Vector3f(a_light->mPosition[0],
//   //   a_light->mPosition[1], a_light->mPosition[2]); l.direction =
//   //   Eigen::Vector3f(
//   //       a_light->mDirection[0], a_light->mDirection[1],
//   //       a_light->mDirection[2]);
//   //   l.color = Eigen::Vector3f(a_light->mColorDiffuse[0],
//   //   a_light->mColorDiffuse[1], a_light->mColorDiffuse[2]);
//   // }
//   Lights lights;

//   lights.lights_count = 1;
//   auto &l = lights.l[0];
//   // l.light_type = LightType::DIRECTIONAL;
//   // l.direction = Eigen::Vector3f(0.0f, -1.0f, 1.0f).normalized();
//   // l.intensity = Eigen::Vector3f(0.8f, 0.8f, 0.8f);

//   l.light_type = LightType::AREA;
//   l.intensity = Eigen::Vector3f(0.8f, 0.8f, 0.2f);
//   l.position[0] = Eigen::Vector4f(-8.0f, 2.4f, -1.0f, 1.0f);
//   l.position[1] = Eigen::Vector4f(-8.0f, 2.4f, 1.0f, 1.0f);
//   l.position[2] = Eigen::Vector4f(-8.0f, 0.4f, 1.0f, 1.0f);
//   l.position[3] = Eigen::Vector4f(-8.0f, 0.4f, -1.0f, 1.0f);
//   return lights;
// }

void processNode(const std::shared_ptr<TransformRelationship> &root_tr,
                 const aiScene *a_scene,
                 const std::vector<std::shared_ptr<StaticMesh>> &meshes,
                 const std::vector<std::shared_ptr<Material>> &materials,
                 std::vector<MeshEntityData> &mesh_entity_datas) {
  std::queue<std::pair<aiNode *, std::shared_ptr<TransformRelationship>>> q;
  q.emplace(std::make_pair(a_scene->mRootNode, root_tr));
  while(!q.empty())
  {
    auto [node, tr] = q.front();
    q.pop();
    memcpy(tr->ltransform.data(), &node->mTransformation,
         sizeof(Eigen::Matrix4f));
    tr->ltransform.transposeInPlace(); // row major to column major
    for (auto i = 0; i < node->mNumMeshes; ++i)
    {
      aiMesh *a_mesh = a_scene->mMeshes[node->mMeshes[i]];
      
      mesh_entity_datas.emplace_back(a_mesh->mName.C_Str(),
                                     meshes[node->mMeshes[i]],
                                     materials[a_mesh->mMaterialIndex],
                                     tr);
      tr->aabb.extend(meshes[node->mMeshes[i]]->getBoundingBox());
    }    
    auto prev_tr = tr;
    for (auto i = 0; i < node->mNumChildren; ++i)
    {
      auto child_tr = std::make_shared<TransformRelationship>();
      child_tr->parent = tr;
      if(i == 0) {        
        prev_tr->child = child_tr;
      } else {
        prev_tr->sibling = child_tr;
      }
      prev_tr = child_tr;
      q.emplace(std::make_pair(node->mChildren[i], child_tr));
    }
  }
}

bool AssimpImporter::import(const URL &url, World *world) {
  Assimp::Importer importer;
  auto path = url.getAbsolute();
  const aiScene *a_scene =
      importer.ReadFile(path, aiProcessPreset_TargetRealtime_Quality |
                                  aiProcess_GenBoundingBoxes);

  std::size_t found = path.find_last_of("/\\");
  std::string dir =
      (found == std::string::npos) ? "./" : path.substr(0, found + 1);

  if (!a_scene) {
    throw std::runtime_error("Assimp import error:" +
                             std::string(importer.GetErrorString()));
  }
  // file_directory_ = path.substr(0, path.find_last_of('/'));
  //  add materials and meshes to scene
  // auto driver = g_engine.getDriver();
  // auto &cmd_buffer_mgr = driver->getThreadLocalCommandBufferManager();
  // auto cmd_buffer = cmd_buffer_mgr.requestCommandBuffer(
  //     VK_COMMAND_BUFFER_LEVEL_PRIMARY);
  //std::cout << "importing cmd buffer: " << cmd_buffer->getHandle() << std::endl;
  std::vector<std::shared_ptr<StaticMesh>> meshes =
      processMeshs(a_scene);
  
  std::vector<std::shared_ptr<Material>> materials =
      processMaterials(a_scene, dir);
  // std::vector<CameraComponent> cameras = processCameras(a_scene);
  //auto lights = processLight(a_scene);
  
  auto scene_tr = std::make_shared<TransformRelationship>();
  std::vector<MeshEntityData> mesh_entity_datas;
  processNode(scene_tr, a_scene, meshes, materials,
              mesh_entity_datas); // TODO add lights process
  world->enqueue(scene_tr, std::move(mesh_entity_datas));
  
  // load the default camera if have
  LOGI("load scene: {}", path.c_str());
  return true;
}

// std::shared_ptr<TransformRelationship>
// AssimpLoader::processNode(const std::shared_ptr<TransformRelationship>
// &parent,
//                           aiNode *node, const aiScene *a_scene, World &scene,
//                           std::vector<std::shared_ptr<StaticMesh>> &meshes,
//                           std::vector<std::shared_ptr<Material>> &materials)
//                           {
//   auto cur_tr = std::make_shared<TransformRelationship>();
//   cur_tr->parent = parent;
//   memcpy(cur_tr->ltransform.data(), &node->mTransformation,
//          sizeof(Eigen::Matrix4f));
//   cur_tr->ltransform.transposeInPlace(); // row major to column major

//   // process all the node's meshes (if any)
//   for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
//     aiMesh *a_mesh = a_scene->mMeshes[node->mMeshes[i]];
//     assert(materials.size() > a_mesh->mMaterialIndex);
//     assert(meshes.size() > node->mMeshes[i]);
//     auto renderable_entt = scene.createRenderableEntity(
//         a_mesh->mName.C_Str(), cur_tr, materials[a_mesh->mMaterialIndex],
//         meshes[node->mMeshes[i]]);
//     cur_tr->aabb.extend(meshes[node->mMeshes[i]]->aabb);
//   }
//   return cur_tr;
// }

// std::vector<CameraComponent>
// AssimpLoader::processCameras(const aiScene *a_scene) {
//   std::vector<CameraComponent> ret_cameras(a_scene->mNumCameras);
//   for (auto i = 0; i < a_scene->mNumCameras; ++i) {
//     auto a_camera = a_scene->mCameras[i];
//     auto &cur_camera = ret_cameras[i];
//     cur_camera.setName(a_camera->mName.C_Str());
//     cur_camera.setClipPlanes(a_camera->mClipPlaneNear,
//     a_camera->mClipPlaneFar); float aspect = a_camera->mAspect < 1e-3 ? 1.0f
//     : a_camera->mAspect; cur_camera.setAspect(aspect);
//     cur_camera.setFovy(a_camera->mHorizontalFOV / aspect);
//     cur_camera.setLookAt(
//         Eigen::Vector3f(a_camera->mPosition.x, a_camera->mPosition.y,
//                         a_camera->mPosition.z),
//         Eigen::Vector3f(a_camera->mUp.x, a_camera->mUp.y, a_camera->mUp.z),
//         Eigen::Vector3f(a_camera->mLookAt.x, a_camera->mLookAt.y,
//                         a_camera->mLookAt.z));
//   }

//   return ret_cameras;
// }

// void loadAndSet(const std::string &dir, const aiScene *a_scene,
//                 aiMaterial *a_mat,
//                 const std::shared_ptr<CommandBuffer> &cmd_buf,
//                 std::shared_ptr<PbrMaterial> &mat) {
//   aiString texture_path;
//   if (AI_SUCCESS ==
//       a_mat->GetTexture(AI_MATKEY_BASE_COLOR_TEXTURE, &texture_path)) {
//     auto a_texture = a_scene->GetEmbeddedTexture(texture_path.C_Str());

//     auto &asset_manager = getDefaultAppContext().gpu_asset_manager;
//     std::shared_ptr<ImageView> img_view =
//         (a_texture != nullptr)
//             ? asset_manager->request<ImageView>(
//                   reinterpret_cast<uint8_t *>(a_texture->pcData),
//                   a_texture->mWidth, cmd_buf)
//             : asset_manager->request<ImageView>(dir + texture_path.C_Str(),
//                                                 cmd_buf);
//     mat->setTexture(BASE_COLOR_TEXTURE_NAME, img_view);
//   } else {
//     aiColor3D value(0.0f, 0.0f, 0.0f);
//     a_mat->Get(AI_MATKEY_COLOR_DIFFUSE, value);
//     mat->setUboParamValue(BASE_COLOR_NAME,
//                           Eigen::Vector4f(value.r, value.g, value.b, 1.0));
//   }

//   // metallic - roughness
//   aiString metallic_tex_path, roughness_tex_path;
//   bool has_m = (AI_SUCCESS == a_mat->GetTexture(AI_MATKEY_ROUGHNESS_TEXTURE,
//                                                 &metallic_tex_path));
//   bool has_r = (AI_SUCCESS == a_mat->GetTexture(AI_MATKEY_ROUGHNESS_TEXTURE,
//                                                 &roughness_tex_path));
//   if (has_m && has_r && metallic_tex_path == roughness_tex_path) {
//     auto a_texture = a_scene->GetEmbeddedTexture(metallic_tex_path.C_Str());

//     auto &asset_manager = getDefaultAppContext().gpu_asset_manager;
//     std::shared_ptr<ImageView> img_view =
//         (a_texture != nullptr)
//             ? asset_manager->request<ImageView>(
//                   reinterpret_cast<uint8_t *>(a_texture->pcData),
//                   a_texture->mWidth, cmd_buf)
//             : asset_manager->request<ImageView>(dir +
//             metallic_tex_path.C_Str(),
//                                                 cmd_buf);
//     mat->setTexture(METALLIC_ROUGHNESS_TEXTURE_NAME, img_view);
//   } else {
//     if (has_m) {
//       auto a_texture =
//       a_scene->GetEmbeddedTexture(metallic_tex_path.C_Str());

//       auto &asset_manager = getDefaultAppContext().gpu_asset_manager;
//       std::shared_ptr<ImageView> img_view =
//           (a_texture != nullptr)
//               ? asset_manager->request<ImageView>(
//                     reinterpret_cast<uint8_t *>(a_texture->pcData),
//                     a_texture->mWidth, cmd_buf)
//               : asset_manager->request<ImageView>(
//                     dir + metallic_tex_path.C_Str(), cmd_buf);
//       mat->setTexture(METALLIC_TEXTURE_NAME, img_view);
//     } else {
//       float value = 0.0f;
//       a_mat->Get(AI_MATKEY_METALLIC_FACTOR, value);
//       mat->setUboParamValue(METALLIC_NAME, value);
//     }
//     if (has_r) {
//       auto a_texture =
//       a_scene->GetEmbeddedTexture(roughness_tex_path.C_Str());

//       auto &asset_manager = getDefaultAppContext().gpu_asset_manager;
//       std::shared_ptr<ImageView> img_view =
//           (a_texture != nullptr)
//               ? asset_manager->request<ImageView>(
//                     reinterpret_cast<uint8_t *>(a_texture->pcData),
//                     a_texture->mWidth, cmd_buf)
//               : asset_manager->request<ImageView>(
//                     dir + roughness_tex_path.C_Str(), cmd_buf);
//       mat->setTexture(ROUGHNESS_TEXTURE_NAME, img_view);
//     } else {
//       float value = 0.0f;
//       a_mat->Get(AI_MATKEY_ROUGHNESS_FACTOR, value);
//       mat->setUboParamValue(ROUGHNESS_NAME, value);
//     }
//   }

//   // specular
//   if (AI_SUCCESS ==
//       a_mat->GetTexture(aiTextureType_SPECULAR, 0, &texture_path)) {
//     auto a_texture = a_scene->GetEmbeddedTexture(texture_path.C_Str());

//     auto &asset_manager = getDefaultAppContext().gpu_asset_manager;
//     std::shared_ptr<ImageView> img_view =
//         (a_texture != nullptr)
//             ? asset_manager->request<ImageView>(
//                   reinterpret_cast<uint8_t *>(a_texture->pcData),
//                   a_texture->mWidth, cmd_buf)
//             : asset_manager->request<ImageView>(dir + texture_path.C_Str(),
//                                                 cmd_buf);
//     mat->setTexture(SPECULAR_TEXTURE_NAME, img_view);
//   } else {
//     float value = 0.5f;
//     a_mat->Get(AI_MATKEY_SPECULAR_FACTOR, value);
//     mat->setUboParamValue(SPECULAR_NAME, value);
//   }

//   // normal map
//   if (AI_SUCCESS ==
//       a_mat->GetTexture(aiTextureType_NORMALS, 0, &texture_path)) {
//     auto a_texture = a_scene->GetEmbeddedTexture(texture_path.C_Str());

//     auto &asset_manager = getDefaultAppContext().gpu_asset_manager;
//     std::shared_ptr<ImageView> img_view =
//         (a_texture != nullptr)
//             ? asset_manager->request<ImageView>(
//                   reinterpret_cast<uint8_t *>(a_texture->pcData),
//                   a_texture->mWidth, cmd_buf)
//             : asset_manager->request<ImageView>(dir + texture_path.C_Str(),
//                                                 cmd_buf);
//     mat->setTexture(NORMAL_TEXTURE_NAME, img_view);
//   }
// }

// std::vector<std::shared_ptr<Material>>
// AssimpLoader::processMaterials(const aiScene *a_scene, const std::string
// &dir,
//                                const std::shared_ptr<CommandBuffer> &cmd_buf)
//                                {
//   auto num_materials = a_scene->mNumMaterials;
//   std::vector<std::shared_ptr<Material>> ret_mats(num_materials);

//   auto driver = getDefaultAppContext().driver;
//   auto gpu_asset_manager = getDefaultAppContext().gpu_asset_manager;

//   for (uint32_t i = 0; i < num_materials; ++i) {
//     auto a_mat = a_scene->mMaterials[i];
//     auto cur_mat = std::make_shared<PbrMaterial>();
//     ret_mats[i] = cur_mat;
//     // aiTextureType_DIFFUSE is same as aiTextureType_BASE_COLOR
//     // diffuse is used for old specular-glossiness workflow
//     // and base color is used for metallic-roughness workflow
//     loadAndSet(dir, a_scene, a_mat, cmd_buf, cur_mat);
//     cur_mat->compile();
//   }
//   return ret_mats;
// }
} // namespace mango