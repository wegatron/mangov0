// #include <iostream>

// #include <assimp/Exporter.hpp>
// #include <assimp/Importer.hpp>
// #include <assimp/postprocess.h>
// #include <assimp/scene.h>
// #include <fstream>
// // #include <entt/entt.hpp>
// // #include <engine/functional/component/material.h>
// // #include <engine/utils/vk/shader_module.h>
// #include <Eigen/Dense>

// void exportVs(const std::string &filename,
//                const Eigen::Matrix<float, 3, Eigen::Dynamic> &fvs)
// {
//   std::ofstream vsFile(filename);

//   if (!vsFile.is_open()) {
//     std::cerr << "Error opening file: " << filename << std::endl;
//     return;
//   }

//   // Write vertices
//   for (int i = 0; i < fvs.cols(); ++i) {
//     vsFile << fvs(0, i) << " " << fvs(1, i) << " " << fvs(2, i) << "\n";
//   }

//   vsFile.close();
// }

// void loadVs(const std::string &filename, Eigen::Matrix<float, 3,
// Eigen::Dynamic> &fvs)
// {
//   std::ifstream vsFile(filename);

//   if (!vsFile.is_open()) {
//     std::cerr << "Error opening file: " << filename << std::endl;
//     return;
//   }

//   // Write vertices
//   for (int i = 0; i < fvs.cols(); ++i) {
//     vsFile >> fvs(0, i) >> fvs(1, i) >> fvs(2, i);
//   }

//   vsFile.close();
// }

// void exportVtk(const std::string &filename,
//                const Eigen::Matrix<float, 3, Eigen::Dynamic> &fvs) {
//   std::ofstream vtkFile(filename);

//   if (!vtkFile.is_open()) {
//     std::cerr << "Error opening file: " << filename << std::endl;
//     return;
//   }

//   // Write header
//   vtkFile << "# vtk DataFile Version 3.0\n";
//   vtkFile << "vtk output\n";
//   vtkFile << "ASCII\n";
//   vtkFile << "DATASET POLYDATA\n";
//   vtkFile << "POINTS " << fvs.cols() << " float\n";

//   // Write vertices
//   for (int i = 0; i < fvs.cols(); ++i) {
//     vtkFile << fvs(0, i) << " " << fvs(1, i) << " " << fvs(2, i) << "\n";
//   }

//   // Write faces
//   int num_faces = fvs.cols() / 3;
//   vtkFile << "POLYGONS " << num_faces << " " << num_faces * 4 << "\n";
//   for (int i = 0; i < num_faces * 3; i += 3) {
//     vtkFile << "3 " << i << " " << i + 1 << " " << i + 2 << "\n";
//   }

//   vtkFile.close();
// }

// void exportObj(const std::string &filename, const int *faces,
//                const float *vertices, int num_vs, int num_faces) {
//   std::ofstream objFile(filename);

//   if (!objFile.is_open()) {
//     std::cerr << "Error opening file: " << filename << std::endl;
//     return;
//   }

//   // Write vertices
//   for (int i = 0; i < num_vs * 3; i += 3) {
//     objFile << "v " << vertices[i] << " " << vertices[i + 1] << " "
//             << vertices[i + 2] << "\n";
//   }

//   // Write faces
//   for (int i = 0; i < num_faces * 3; i += 3) {
//     objFile << "f " << faces[i] + 1 << " " << faces[i + 1] + 1 << " "
//             << faces[i + 2] + 1 << "\n";
//   }

//   objFile.close();
// }

// int exportFvsObj()
// {
//   Eigen::Matrix<float, 3, Eigen::Dynamic> fvs_0;
//   fvs_0.resize(3, 192282);
//   Eigen::Matrix<float, 3, Eigen::Dynamic> cur_fvs;
//   cur_fvs.resize(3, 192282);
//   for (auto file_id = 2; file_id < 99; ++file_id) {
//     std::string file_path =
//         "/home/wegatron/win-data/data/FLAME/mh_shapes/flame_" +
//         std::to_string(file_id) + ".fbx";
//     std::cout << "file_path:" << file_path << std::endl;
//     Assimp::Importer importer;
//     const aiScene *a_scene = importer.ReadFile(file_path.c_str(),
//     aiProcessPreset_TargetRealtime_Fast); if (!a_scene) {
//       std::cerr << "Error: " << importer.GetErrorString() << std::endl;
//       return 1;
//     }

//     // merge all meshes into one
//     auto a_meshes = a_scene->mMeshes;
//     size_t fvs_idx = -1;
//     for (auto mesh_i = 0; mesh_i < a_scene->mNumMeshes; ++mesh_i) {
//       auto &tmp_mesh = a_meshes[mesh_i];
//       auto &vertices = a_meshes[mesh_i]->mVertices;
//       for (auto fi = 0; fi < tmp_mesh->mNumFaces; ++fi) {
//         ++fvs_idx;
//         cur_fvs(0, fvs_idx) = vertices[tmp_mesh->mFaces[fi].mIndices[0]].x;
//         cur_fvs(1, fvs_idx) = vertices[tmp_mesh->mFaces[fi].mIndices[0]].y;
//         cur_fvs(2, fvs_idx) = vertices[tmp_mesh->mFaces[fi].mIndices[0]].z;

//         ++fvs_idx;
//         cur_fvs(0, fvs_idx) = vertices[tmp_mesh->mFaces[fi].mIndices[1]].x;
//         cur_fvs(1, fvs_idx) = vertices[tmp_mesh->mFaces[fi].mIndices[1]].y;
//         cur_fvs(2, fvs_idx) = vertices[tmp_mesh->mFaces[fi].mIndices[1]].z;

//         ++fvs_idx;
//         cur_fvs(0, fvs_idx) = vertices[tmp_mesh->mFaces[fi].mIndices[2]].x;
//         cur_fvs(1, fvs_idx) = vertices[tmp_mesh->mFaces[fi].mIndices[2]].y;
//         cur_fvs(2, fvs_idx) = vertices[tmp_mesh->mFaces[fi].mIndices[2]].z;
//       }
//     }
//     if (file_id == 2) {
//       fvs_0 = cur_fvs;
//     }

//     exportVtk("debug/test_" + std::to_string(file_id) + ".vtk", cur_fvs);
//     exportVtk("debug/testm_" + std::to_string(file_id) + ".vtk",
//               (fvs_0 + cur_fvs) * 0.5);
//   }
//   return 0;
// }

// #define EXPORT_ORDER 1

// Eigen::Matrix<int, 6, 1> EVector6i(int a, int b, int c, int d, int e, int f)
// {
//   Eigen::Matrix<int, 6, 1> ret;
//   ret << a, b, c, d, e, f;
//   return ret;
// }

// Eigen::Matrix<int, 7, 1> EVector7i(int a, int b, int c, int d, int e, int f,
// int g)
// {
//   Eigen::Matrix<int, 7, 1> ret;
//   ret << a, b, c, d, e, f, g;
//   return ret;
// }

// int exportFvsObj2()
// {
//   Eigen::Matrix<float, 3, Eigen::Dynamic> fvs_0;
//   fvs_0.resize(3, 192282);
//   Eigen::Matrix<float, 3, Eigen::Dynamic> cur_fvs;
//   cur_fvs.resize(3, 192282);

//   std::vector<Eigen::Vector3f> tmp_fvs;
//   std::vector < Eigen::Vector4i> keys;

//   for (auto file_id = 0; file_id < 1; ++file_id) {
//     #if EXPORT_ORDER
//     std::string file_path =
//         "/home/wegatron/opt/blender-3.6.1-linux-x64/3.6/scripts/addons/mh_anim/data/mh_template1.fbx";
//     #else
//     std::string file_path =
//         "/home/wegatron/win-data/data/FLAME/mh_shapes/flame_" +
//         std::to_string(file_id) + ".fbx";
//     #endif
//     std::cout << "file_path:" << file_path << std::endl;
//     Assimp::Importer importer;
//     const aiScene *a_scene = importer.ReadFile(file_path.c_str(), 0);
//     //aiProcessPreset_TargetRealtime_Fast); if (!a_scene) {
//       std::cerr << "Error: " << importer.GetErrorString() << std::endl;
//       return 1;
//     }

//     // merge all meshes into one
//     auto a_meshes = a_scene->mMeshes;
//     size_t vi_offset = 0;
//     size_t nv = 0;
//     #if EXPORT_ORDER
//     std::ofstream t_order_ofs("debug/template_order.txt");
//     #endif

//     std::ofstream keys_ofs("debug/flame_keys.txt");
//     for (auto mesh_i = 0; mesh_i < a_scene->mNumMeshes; ++mesh_i) {
//       auto &tmp_mesh = a_meshes[mesh_i];
//       auto &vertices = tmp_mesh->mVertices;
//       auto &uvs = tmp_mesh->mTextureCoords[0];
//       // std::cout << tmp_mesh->mNumVertices <<  " -- " <<
//       3*tmp_mesh->mNumFaces
//       // << std::endl;
//       nv += tmp_mesh->mNumVertices;

//       tmp_fvs.resize(tmp_mesh->mNumVertices);
//       keys.resize(tmp_mesh->mNumVertices/3);
//       for (auto vi = 0; vi < tmp_mesh->mNumVertices; ++vi) {
//         tmp_fvs[vi] = Eigen::Vector3f(vertices[vi].x, vertices[vi].y,
//         vertices[vi].z); if(vi % 3 == 0)
//           keys[vi/3] = Eigen::Vector4i(uvs[vi].x * 1000, uvs[vi].y * 1000,
//           vertices[vi].x * 1000, vi/3);
//         else
//           keys[vi/3].head(3) += Eigen::Vector3i(uvs[vi].x * 1000, uvs[vi].y *
//           1000, vertices[vi].x * 1000);
//       }

//       std::sort(keys.begin(), keys.end(), [mesh_i](const Eigen::Vector4i &a,
//       const Eigen::Vector4i &b) {
//         if(a.x() == b.x())
//         {
//           if(a.y() == b.y())
//           {
//             return a.z() < b.z();
//           }
//           return a.y() < b.y();
//         }
//         return a.x() < b.x();
//       });

//       // export keys
//       for (auto fi = 0; fi < tmp_mesh->mNumVertices/3; ++fi) {
//         keys_ofs << (keys[fi] + Eigen::Vector4i(0, 0, 0,
//         vi_offset/3)).transpose() << "\n";
//       }

//       // update
//       for (auto fi = 0; fi< tmp_mesh->mNumVertices/3; ++fi)
//       {
//         auto &key = keys[fi];
//         assert(key.w() * 3 + 2 < tmp_fvs.size());
//         cur_fvs(0, vi_offset + fi * 3) = tmp_fvs[key.w() * 3].x();
//         cur_fvs(1, vi_offset + fi*3) = tmp_fvs[key.w()*3].y();
//         cur_fvs(2, vi_offset + fi*3) = tmp_fvs[key.w()*3].z();

//         cur_fvs(0, vi_offset + fi*3+1) = tmp_fvs[key.w()*3+1].x();
//         cur_fvs(1, vi_offset + fi*3+1) = tmp_fvs[key.w()*3+1].y();
//         cur_fvs(2, vi_offset + fi*3+1) = tmp_fvs[key.w()*3+1].z();

//         cur_fvs(0, vi_offset + fi*3+2) = tmp_fvs[key.w()*3+2].x();
//         cur_fvs(1, vi_offset + fi*3+2) = tmp_fvs[key.w()*3+2].y();
//         cur_fvs(2, vi_offset + fi*3+2) = tmp_fvs[key.w()*3+2].z();

//         #if EXPORT_ORDER
//         t_order_ofs << vi_offset/3 + key.z() << " ";
//         #endif
//       }

//       exportVtk("debug/mesh_part_" + std::to_string(mesh_i) + ".vtk",
//       cur_fvs.block(0, vi_offset, 3, tmp_mesh->mNumVertices));

//       vi_offset += tmp_mesh->mNumVertices;
//       std::cout << vi_offset/3 << std::endl;
//     }
//     #if EXPORT_ORDER
//     t_order_ofs.close();
//     #endif
//     if (file_id == 0) {
//       fvs_0 = cur_fvs;
//     }

//     keys_ofs.close();
//     std::cout << "full nv:" << nv << std::endl;

//     Eigen::Matrix<float, 3, Eigen::Dynamic> t_fvs = cur_fvs.block(
//         0, a_meshes[0]->mNumVertices, 3, a_meshes[1]->mNumVertices);
//     // exportVtk("debug/test_" + std::to_string(file_id) + ".vtk",
//     //           cur_fvs.block(0, a_meshes[1]->mNumVertices, 3,
//     a_meshes[1]->mNumVertices));
//     // exportVtk("debug/testm_" + std::to_string(file_id) + ".vtk",
//     //           (fvs_0 + cur_fvs).block(0, a_meshes[0]->mNumVertices, 3,
//     a_meshes[1]->mNumVertices) * 0.5);

//     exportVtk("debug/test_" + std::to_string(file_id) + ".vtk",
//               cur_fvs);
//     exportVtk("debug/testm_" + std::to_string(file_id) + ".vtk",
//               (fvs_0 + cur_fvs) * 0.5);
//     exportVs("debug/test_" + std::to_string(file_id) + ".txt", cur_fvs);
//   }

//   return 0;
// }

// void MergeTest()
// {
//   Eigen::Matrix<float, 3, Eigen::Dynamic> fvs_0;
//   fvs_0.resize(3, 192282);
//   loadVs("debug/blender_sorted_fvs.txt", fvs_0);

//   Eigen::Matrix<float, 3, Eigen::Dynamic> t_fvs;
//   t_fvs.resize(3, 192282);
//   loadVs("debug/blender_sorted_fvs1.txt", t_fvs);

//   //exportVtk("debug/template_order.vtk", t_fvs);

//   exportVtk("debug/template_m.vtk", (fvs_0 + t_fvs) * 0.5);
// }

// /**
//  * have a precision problem when export and loading the exported uvs from
//  blender
// */
// void sortBlenderTemplateMesh(const std::string &fbx_file, const std::string
// &uvs_file, const std::string &fvs_file)
// {
//   Assimp::Importer importer;
//   const aiScene *a_scene = importer.ReadFile(fbx_file.c_str(), 0);
//   if (!a_scene) {
//     std::cerr << "Error: " << importer.GetErrorString() << std::endl;
//     return;
//   }

//   int full_nf = 0;
//   auto a_meshes = a_scene->mMeshes;
//   std::vector<int> nfs(a_scene->mNumMeshes);
//   for (auto mesh_i = 0; mesh_i < a_scene->mNumMeshes; ++mesh_i) {
//     auto &tmp_mesh = a_meshes[mesh_i];
//     //std::cout << "nf:" << a_meshes[mesh_i]->mNumFaces << std::endl;
//     std::cout << full_nf << std::endl;
//     nfs[mesh_i] = a_meshes[mesh_i]->mNumFaces;
//     full_nf += a_meshes[mesh_i]->mNumFaces;
//   }

//   std::vector<Eigen::Vector3i> keys;
//   std::ifstream ifs(uvs_file);
//   if (!ifs.is_open()) {
//     std::cerr << "Error opening file: " << uvs_file << std::endl;
//     return;
//   }
//   keys.resize(full_nf);
//   for (auto fi = 0; fi < full_nf; ++fi) {
//     ifs >> keys[fi].x() >> keys[fi].y() >> keys[fi].z();
//   }
//   ifs.close();

//   size_t offset = 0;
//   for (auto mesh_i = 0; mesh_i < nfs.size(); ++mesh_i) {
//     std::sort(keys.begin() + offset, keys.begin() + offset + nfs[mesh_i],
//     [](const Eigen::Vector3i &a, const Eigen::Vector3i &b) {
//       if(a.x() == b.x())
//       {
//         if(a.y() == b.y())
//         {
//           return a.z() < b.z();
//         }
//         return a.y() < b.y();
//       }
//       return a.x() < b.x();
//     });
//     offset += nfs[mesh_i];
//   }

//   // read fvs
//   Eigen::Matrix<float, 3, Eigen::Dynamic> fvs;
//   fvs.resize(3, full_nf * 3);
//   std::ifstream ifs2(fvs_file);
//   if (!ifs2.is_open()) {
//     std::cerr << "Error opening file: " << fvs_file << std::endl;
//     return;
//   }
//   for (auto fi = 0; fi < full_nf * 3; ++fi) {
//     ifs2 >> fvs(0, fi) >> fvs(1, fi) >> fvs(2, fi);
//   }
//   ifs2.close();

//   // update fvs
//   Eigen::Matrix<float, 3, Eigen::Dynamic> sorted_fvs;
//   sorted_fvs.resize(3, full_nf * 3);
//   for (auto fi = 0; fi < full_nf; ++fi) {
//     sorted_fvs.col(fi * 3) = fvs.col(keys[fi].z() * 3);
//     sorted_fvs.col(fi * 3 + 1) = fvs.col(keys[fi].z() * 3 + 1);
//     sorted_fvs.col(fi * 3 + 2) = fvs.col(keys[fi].z() * 3 + 2);
//   }

//   // export keys
//   std::ofstream ofs2("debug/template_keys.txt");
//   if (!ofs2.is_open()) {
//     std::cerr << "Error opening file: debug/template_keys.txt" << std::endl;
//     return;
//   }
//   for (auto fi = 0; fi < full_nf; ++fi) {
//     ofs2 << keys[fi].transpose() << "\n";
//   }
//   ofs2.close();

//   std::ofstream ofs("debug/template_order_fvs.txt");
//   if (!ofs.is_open()) {
//     std::cerr << "Error opening file: debug/template_order_fvs.txt" <<
//     std::endl; return;
//   }
//   for (auto fi = 0; fi < full_nf * 3; ++fi) {
//     ofs << sorted_fvs(0, fi) << " " << sorted_fvs(1, fi) << " " <<
//     sorted_fvs(2, fi) << "\n";
//   }
//   ofs.close();

//   // export order
//   std::ofstream ofs3("debug/template_order.txt");
//   if (!ofs3.is_open()) {
//     std::cerr << "Error opening file: debug/template_order.txt" << std::endl;
//     return;
//   }
//   for (auto fi = 0; fi < full_nf; ++fi) {
//     ofs3 << keys[fi].z() << " ";
//   }
//   ofs3.close();
// }

// int main(int argc, char const *argv[]) {
//   //exportFvsObj();
//   exportFvsObj2(); // sort faces by uv
//   //
//   sortBlenderTemplateMesh("/home/wegatron/opt/blender-3.6.1-linux-x64/3.6/scripts/addons/mh_anim/data/mh_template.fbx",
//   //   "/home/wegatron/win-data/workspace/mango/debug/template_f_uvs.txt",
//   //   "/home/wegatron/win-data/workspace/mango/debug/template_fvs.txt");
//   //MergeTest();
//   // for (auto file_id = 0; file_id < 99; ++file_id) {
//   //   std::string file_path =
//   //       "/home/wegatron/win-data/data/FLAME/mh_shapes/flame_" +
//   //       std::to_string(file_id) + ".fbx";
//   //   std::cout << "file_path:" << file_path << std::endl;
//   //   Assimp::Importer importer;
//   //   const aiScene *a_scene = importer.ReadFile(file_path.c_str(), 0);
//   //   if (!a_scene) {
//   //     std::cerr << "Error: " << importer.GetErrorString() << std::endl;
//   //     return 1;
//   //   }

//   //   // merge all meshes into one
//   //   auto a_meshes = a_scene->mMeshes;
//   //   size_t fvs_idx = -1;
//   //   size_t nv = 0;
//   //   for (auto mesh_i = 0; mesh_i < a_scene->mNumMeshes; ++mesh_i) {
//   //     auto &tmp_mesh = a_meshes[mesh_i];
//   //     auto &vertices = a_meshes[mesh_i]->mVertices;
//   //     nv += a_meshes[mesh_i]->mNumVertices;
//   //     std::cout << a_meshes[mesh_i]->mNumVertices << std::endl;
//   //   }

//   //   std::cout << "full nv:" << nv << std::endl;
//   // }

//   return 0;
// }

#include <engine/functional/component/light.h>
#include <iostream>

int main(int argc, char const *argv[]) {
  /* code */
  mango::Light light_ub;
  // std::cout << reinterpret_cast<size_t>(&(light_ub.direction)) -
  // reinterpret_cast<size_t>(&(light_ub.type)) << std::endl; std::cout <<
  // reinterpret_cast<size_t>(&(light_ub.color_intensity)) -
  // reinterpret_cast<size_t>(&(light_ub.direction)) << std::endl; std::cout <<
  // reinterpret_cast<size_t>(&(light_ub.position_falloff)) -
  // reinterpret_cast<size_t>(&(light_ub.color_intensity)) << std::endl;
  std::cout << sizeof(light_ub) << std::endl;
  // std::cout << alignof(light_ub.type) << std::endl;
  return 0;
}
