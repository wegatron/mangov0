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

void StaticMesh::inflate() {
  calcBoundingBox();
  // upload to gpu
  auto driver = g_engine.getDriver();
  static_assert(sizeof(StaticVertex) == 8 * sizeof(float) && sizeof(float) == 4,
                "StaticVertex size is not 8 * sizeof(float)");
  vertex_buffer_ = std::make_shared<Buffer>(
      driver, vertices_.size() * sizeof(StaticVertex), 
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      0,
      0,
      VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

  auto cmd_buffer = driver->getThreadLocalCommandBufferManager().requestCommandBuffer(
      VK_COMMAND_BUFFER_LEVEL_PRIMARY);
  vertex_buffer_->updateByStaging(vertices_.data(), vertices_.size() * sizeof(StaticVertex),
                      0, cmd_buffer);

  // buffer: indices data triangle faces
  index_buffer_ = std::make_shared<Buffer>(
      driver, indices_.size() * sizeof(uint32_t),
      VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 0,
      0,
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