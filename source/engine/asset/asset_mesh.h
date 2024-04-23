#pragma once

#include <Eigen/Geometry>
#include <engine/asset/asset.h>
#include <engine/utils/vk/buffer.h>

namespace mango {
class CommandBuffer;
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

  void setSubMeshs(const std::vector<SubMesh> &sub_meshes) {
    sub_meshes_ = sub_meshes;
  }

  void setIndices(const std::vector<uint32_t> &indices) { indices_ = indices; }

  const Eigen::AlignedBox3f &getBoundingBox() const { return bounding_box_; }

  virtual void inflate(
      const std::shared_ptr<CommandBuffer> &cmd_buffer); //!< upload data to gpu

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

  void setVertices(const std::vector<StaticVertex> &vertices) {
    vertices_ = vertices;
  }

  void inflate(const std::shared_ptr<CommandBuffer> &cmd_buffer) override;

private:
  std::vector<StaticVertex> vertices_;
};

} // namespace mango