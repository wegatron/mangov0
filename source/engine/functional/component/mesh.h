#pragma once

#include <Eigen/Geometry>
#include <engine/utils/vk/buffer.h>

namespace mango {

struct SubMesh {
  uint32_t index_offset;
  uint32_t index_count;
  // std::shared_ptr<Material> m_material;
};
class Mesh {
public:
  Mesh() = default;
  ~Mesh() = default;

  virtual void calcBoundingBox() = 0;

  std::shared_ptr<Buffer> getVertexBuffer() const { return vertex_buffer_; }
  std::shared_ptr<Buffer> getIndexBuffer() const { return index_buffer_; }
  const std::vector<SubMesh> &getSubMeshs() const { return sub_meshes_; }
  // std::vector<uint32_t>

protected:
  std::vector<SubMesh> sub_meshes_; //!< submesh: index offset, index
                                    // count, vertex offset, vertex count
  std::vector<uint32_t> indices_;   //!< indices data on cpu

  std::shared_ptr<Buffer> vertex_buffer_;
  std::shared_ptr<Buffer> index_buffer_;
  Eigen::AlignedBox3f bounding_box_;
};
struct StaticVertex {
  Eigen::Vector3f position;
  Eigen::Vector3f normal;
  Eigen::Vector2f uv;
};

class StaticMesh : public Mesh {
public:
  StaticMesh() = default;
  ~StaticMesh() = default;

  void calcBoundingBox() override;

private:
  std::vector<StaticVertex> vertices_;
};

} // namespace mango