#include "asset_mesh.h"
#include <engine/functional/global/engine_context.h>
#include <engine/utils/vk/commands.h>

namespace mango {
void StaticMesh::calcBoundingBox() {
  bounding_box_.setEmpty();
  for (auto &vertex : vertices_) {
    bounding_box_.extend(vertex.position);
  }
}

void StaticMesh::inflate(const std::shared_ptr<CommandBuffer> &cmd_buffer) {
  calcBoundingBox();
  // upload to gpu
  auto driver = g_engine.getDriver();
  static_assert(sizeof(StaticVertex) == 8 * sizeof(float) && sizeof(float) == 4,
                "StaticVertex size is not 8 * sizeof(float)");
  vertex_buffer_ = std::make_shared<Buffer>(
      driver, 0, vertices_.size() * sizeof(StaticVertex),
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 0,
      VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

  vertex_buffer_->updateByStaging(vertices_.data(), vertices_.size() * sizeof(StaticVertex),
                      0, cmd_buffer);

  // buffer: indices data triangle faces
  index_buffer_ = std::make_shared<Buffer>(
      driver, 0, indices_.size() * sizeof(uint32_t),
      VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 0,
      VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
  // upload data to buffer
  index_buffer_->updateByStaging(indices_.data(), indices_.size() * sizeof(uint32_t), 0,
                      cmd_buffer);
}

void StaticMesh::load(const URL &url) {
  // deserialize from file
  // ...
  assert(false);
}
} // namespace mango