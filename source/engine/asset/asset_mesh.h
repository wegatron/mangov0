#pragma once

#include <Eigen/Geometry>
#include <engine/asset/asset.h>
#include <engine/utils/vk/buffer.h>

namespace mango {

/**
 * @brief submesh share one vertex array, with specified index offset.
 * different submesh may have different material
 */
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
  Eigen::AlignedBox3f bounding_box_;

  // gpu data
  std::shared_ptr<Buffer> vertex_buffer_;
  std::shared_ptr<Buffer> index_buffer_;
};
struct StaticVertex {
  Eigen::Vector3f position;
  Eigen::Vector3f normal;
  Eigen::Vector2f uv;
};

class StaticMesh : public Mesh, public Asset {
public:
  StaticMesh() { asset_type_ = EAssetType::STATICMESH; };
  ~StaticMesh() = default;

  void calcBoundingBox() override;

  void load(const URL &url) override; //!< deserialize from file

private:
  std::vector<StaticVertex> vertices_;
};

} // namespace mango