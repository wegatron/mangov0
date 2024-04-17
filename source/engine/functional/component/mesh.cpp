#include "mesh.h"

namespace mango {
void StaticMesh::calcBoundingBox() {
  bounding_box_.setEmpty();
  for (auto &vertex : vertices_) {
    bounding_box_.extend(vertex.position);
  }
}
} // namespace mango