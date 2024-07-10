#include <engine/asset/assimp_importer.h>

#include <Eigen/Dense>
#include <engine/asset/asset_material.h>
#include <engine/asset/asset_mesh.h>
#include <engine/functional/component/component_camera.h>
#include <engine/functional/component/component_transform.h>
#include <engine/functional/global/engine_context.h>
#include <engine/functional/world/world.h>
#include <engine/utils/base/macro.h>
#include <engine/utils/vk/commands.h>
#include <queue>
#include <shaders/include/shader_structs.h>

namespace mango {

std::vector<std::shared_ptr<StaticMesh>> processMeshs(const aiScene *a_scene) {
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
    ret_meshes[i]->setSubMeshs({{nf * 3, 0}});
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
processMaterials(const aiScene *a_scene, const std::string &dir) {
  std::vector<std::shared_ptr<Material>> ret_mats(a_scene->mNumMaterials);
  for (uint32_t i = 0; i < a_scene->mNumMaterials; ++i) {
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
    if (AI_SUCCESS ==
        a_mat->GetTexture(aiTextureType_NORMALS, 0, &texture_path)) {
      auto a_texture = a_scene->GetEmbeddedTexture(texture_path.C_Str());
      std::string texture_path_str = dir + std::string(texture_path.C_Str());
      cur_mat->setNormalTexture(
          loadTexture(a_texture, texture_path_str.c_str()));
    }
    if (AI_SUCCESS ==
        a_mat->GetTexture(aiTextureType_EMISSIVE, 0, &texture_path)) {
      u_material.emissive_type = static_cast<uint32_t>(ParamType::Texture);
      auto a_texture = a_scene->GetEmbeddedTexture(texture_path.C_Str());
      std::string texture_path_str = dir + std::string(texture_path.C_Str());
      cur_mat->setEmissiveTexture(
          loadTexture(a_texture, texture_path_str.c_str()));
    } else {
      u_material.emissive_type =
          static_cast<uint32_t>(ParamType::CONSTANT_VALUE);
      aiColor3D value(0.0f, 0.0f, 0.0f);
      a_mat->Get(AI_MATKEY_COLOR_EMISSIVE, value);
      u_material.emissive_color =
          Eigen::Vector4f(value.r, value.g, value.b, 1.0f);
    }

    // assert metallic and roughness share the same texture
    if (AI_SUCCESS ==
        a_mat->GetTexture(AI_MATKEY_ROUGHNESS_TEXTURE, &texture_path)) {
      u_material.metallic_roughness_occlution_type =
          static_cast<uint32_t>(ParamType::Texture);
      auto a_texture = a_scene->GetEmbeddedTexture(texture_path.C_Str());
      std::string texture_path_str = dir + std::string(texture_path.C_Str());
      cur_mat->setMetallicRoughnessOcclutionTexture(
          loadTexture(a_texture, texture_path_str.c_str()));
    } else {
      u_material.metallic_roughness_occlution_type =
          static_cast<uint32_t>(ParamType::CONSTANT_VALUE);
      float value = 0.0f;
      a_mat->Get(AI_MATKEY_METALLIC_FACTOR, value);
      u_material.metallic_roughness_occlution[0] = value;
      a_mat->Get(AI_MATKEY_ROUGHNESS_FACTOR, value);
      u_material.metallic_roughness_occlution[1] = value;
    }
    cur_mat->inflate();
  }
  return ret_mats;
}

std::pair<ULighting, std::vector<std::tuple<const char *, uint16_t, uint16_t>>>
processLights(const aiScene *a_scene) {
  ULighting lights;
  std::vector<std::tuple<const char *, uint16_t, uint16_t>> light_nodes_info;
  light_nodes_info.reserve(a_scene->mNumLights);
  for (auto i = 0; i < a_scene->mNumLights; ++i) {
    auto a_light = a_scene->mLights[i];
    switch (a_light->mType) {
    case aiLightSource_DIRECTIONAL: {
      auto &light_num = lights.light_num[LightType::DIRECTIONAL];
      if (light_num < MAX_DIRECTIONAL_LIGHT_NUM) {
        auto &l = lights.directional_lights[light_num];
        l.direction =
            Eigen::Vector4f(a_light->mDirection[0], a_light->mDirection[1],
                            a_light->mDirection[2], 0.0f);
        l.illuminance = Eigen::Vector4f(a_light->mColorDiffuse[0],
                                        a_light->mColorDiffuse[1],
                                        a_light->mColorDiffuse[2], 1.0f);
        light_nodes_info.emplace_back(
            a_light->mName.C_Str(),
            static_cast<uint16_t>(LightType::DIRECTIONAL), light_num);
        ++light_num;
      } else {
        LOGW("Directional light number in imported scene exceeds the limit.");
      }
    } break;
    case aiLightSource_POINT: {
      auto &light_num = lights.light_num[LightType::POINT];
      if (light_num < MAX_POINT_LIGHT_NUM) {
        auto &l = lights.point_lights[light_num];
        l.position =
            Eigen::Vector4f(a_light->mPosition[0], a_light->mPosition[1],
                            a_light->mPosition[2], 1.0f);
        l.luminous_intensity = Eigen::Vector4f(a_light->mColorDiffuse[0],
                                               a_light->mColorDiffuse[1],
                                               a_light->mColorDiffuse[2], 1.0f);
        light_nodes_info.emplace_back(a_light->mName.C_Str(),
                                      static_cast<uint16_t>(LightType::POINT),
                                      light_num);
        ++light_num;
      } else {
        LOGW("Point light number in imported scene exceeds the limit.");
      }
    } break;
    default:
      LOGW("Unsupported light type.");
      break;
    }
  }
  return {lights, light_nodes_info};
}

std::pair<std::vector<MeshEntityData>, std::vector<LightEntityData>>
processNode(const std::shared_ptr<TransformRelationship> &root_tr,
            const aiScene *a_scene,
            const std::vector<std::shared_ptr<StaticMesh>> &meshes,
            const std::vector<std::shared_ptr<Material>> &materials,
            const std::vector<std::tuple<const char *, uint16_t, uint16_t>>
                &light_nodes_info) {
  std::vector<MeshEntityData> mesh_entity_datas;
  std::vector<LightEntityData> light_entity_datas;
  std::queue<std::pair<aiNode *, std::shared_ptr<TransformRelationship>>> q;
  q.emplace(std::make_pair(a_scene->mRootNode, root_tr));
  while (!q.empty()) {
    auto [node, tr] = q.front();
    q.pop();
    auto light_node = std::find_if(
        light_nodes_info.begin(), light_nodes_info.end(),
        [&node](const std::tuple<const char *, uint16_t, uint16_t> &info) {
          return strcmp(node->mName.C_Str(), std::get<0>(info)) == 0;
        });
    if (light_node != light_nodes_info.end()) {
      light_entity_datas.emplace_back(node->mName.C_Str(), tr,
                                      std::get<1>(*light_node),
                                      std::get<2>(*light_node));
    }
    memcpy(tr->ltransform.data(), &node->mTransformation,
           sizeof(Eigen::Matrix4f));
    tr->ltransform.transposeInPlace(); // row major to column major
    for (auto i = 0; i < node->mNumMeshes; ++i) {
      aiMesh *a_mesh = a_scene->mMeshes[node->mMeshes[i]];

      mesh_entity_datas.emplace_back(a_mesh->mName.C_Str(),
                                     meshes[node->mMeshes[i]],
                                     materials[a_mesh->mMaterialIndex], tr);
      tr->aabb.extend(meshes[node->mMeshes[i]]->getBoundingBox());
    }
    auto prev_tr = tr;
    for (auto i = 0; i < node->mNumChildren; ++i) {
      auto child_tr = std::make_shared<TransformRelationship>();
      child_tr->parent = tr;
      if (i == 0) {
        prev_tr->child = child_tr;
      } else {
        prev_tr->sibling = child_tr;
      }
      prev_tr = child_tr;
      q.emplace(std::make_pair(node->mChildren[i], child_tr));
    }
  }
  return {mesh_entity_datas, light_entity_datas};
}

bool AssimpImporter::import(const URL &url, World *world) {
  Assimp::Importer importer;
  auto path = url.getAbsolute();
  const aiScene *a_scene = importer.ReadFile(
      path, aiProcessPreset_TargetRealtime_Quality |
                aiProcess_GenBoundingBoxes | aiProcess_FlipUVs);

  std::size_t found = path.find_last_of("/\\");
  std::string dir =
      (found == std::string::npos) ? "./" : path.substr(0, found + 1);

  if (!a_scene) {
    throw std::runtime_error("Assimp import error:" +
                             std::string(importer.GetErrorString()));
  }
  std::vector<std::shared_ptr<StaticMesh>> meshes = processMeshs(a_scene);

  std::vector<std::shared_ptr<Material>> materials =
      processMaterials(a_scene, dir);
  auto [lights, light_nodes_info] = processLights(a_scene);

  auto scene_tr = std::make_shared<TransformRelationship>();
  auto [mesh_entity_datas, light_entity_datas] =
      processNode(scene_tr, a_scene, meshes, materials, light_nodes_info);
  world->enqueue(scene_tr, std::move(mesh_entity_datas),
                 std::move(light_entity_datas), lights);
  // load the default camera if have
  LOGI("load scene: {}", path.c_str());
  return true;
}
} // namespace mango