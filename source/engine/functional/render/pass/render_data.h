#pragma once

#include <engine/utils/vk/buffer.h>
#include <shaders/include/ubo_structures.h>

namespace mango {

struct StaticMeshRenderData {
  std::shared_ptr<Buffer> vertex_buffer; //!< vertex buffer 3 float position | 3
                                         //!< float normal | 2 float uv
  std::shared_ptr<Buffer> index_buffer;  //!< index buffer uint32_t
  TransformPCO transform_pco;
};

struct RenderData {
  StaticMeshRenderData static_mesh_render_data;
};

} // namespace mango