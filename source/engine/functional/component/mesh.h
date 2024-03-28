#pragma once

#include <Eigen/Geometry>
#include <engine/utils/vk/buffer.h>

namespace mango {

struct VertexBuffer {
  std::shared_ptr<Buffer> buffer;
  uint32_t offset;
  uint32_t stride;
  uint32_t vertex_count;
  VkFormat data_type;
};

struct IndexBuffer {
  std::shared_ptr<Buffer> buffer;
  uint32_t offset;
  uint32_t index_count; // number of vertices
  VkIndexType data_type;
  VkPrimitiveTopology primitive_type;
};

struct StaticMesh {
  VertexBuffer vertices;
  VertexBuffer normals;
  VertexBuffer texture_coords;
  IndexBuffer faces;
  Eigen::AlignedBox3f aabb;
};

} // namespace mango