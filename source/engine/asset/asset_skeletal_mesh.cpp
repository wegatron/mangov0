#include <engine/asset/asset_skeletal_mesh.h>
#include <engine/functional/global/engine_context.h>


namespace mango {
void SkeletalMesh::inflate() {
  auto driver = g_engine.getDriver();
  auto cmd_buffer = driver->getThreadLocalCommandBufferManager().requestCommandBuffer(
      VK_COMMAND_BUFFER_LEVEL_PRIMARY);
  vertex_buffer_ = std::make_shared<Buffer>(
      driver,
      vertices_.size() * sizeof(SkeletalVertex),
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      0, 0,
      VMA_MEMORY_USAGE_GPU_ONLY);
  vertex_buffer_->updateByStaging(vertices_.data(),
                                  vertices_.size() * sizeof(SkeletalVertex), 0,
                                  cmd_buffer);
  index_buffer_ = std::make_shared<Buffer>(
      driver,
      indices_.size() * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      0, 0,
      VMA_MEMORY_USAGE_GPU_ONLY);
  index_buffer_->updateByStaging(indices_.data(),
                                 indices_.size() * sizeof(uint32_t), 0,
                                 cmd_buffer);
}

void SkeletalMesh::calcBoundingBox() {
  bounding_box_.setEmpty();
  for (auto &vertex : vertices_) {
    bounding_box_.extend(vertex.position);
  }
}

} // namespace mango