#pragma once

#include <engine/asset/asset_mesh.h>

namespace mango {
class StaticMeshComponent {
public:
  StaticMeshComponent() = default;
  ~StaticMeshComponent() = default;
  void setStaticMesh(const std::shared_ptr<StaticMesh> &static_mesh) {
    static_mesh_ = static_mesh;
  }
  auto getStaticMesh() const { return static_mesh_; }

private:
  std::shared_ptr<StaticMesh> static_mesh_;
};
} // namespace mango