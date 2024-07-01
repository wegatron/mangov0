#pragma once

#include <engine/utils/vk/buffer.h>
#include <shaders/include/shader_structs.h>

namespace mango {
class DescriptorSet;
struct StaticMeshRenderData {
  std::shared_ptr<Buffer> vertex_buffer; //!< vertex buffer 3 float position | 3
                                         //!< float normal | 2 float uv
  std::shared_ptr<Buffer> index_buffer;  //!< index buffer uint32_t  
  std::shared_ptr<DescriptorSet> material_descriptor_set;
  VkPrimitiveTopology topology;
  TransformPCO transform_pco;
  std::vector<uint32_t> index_counts;
  std::vector<uint32_t> first_index;
};

struct RenderData {
  std::vector<StaticMeshRenderData> static_mesh_render_data;
};

} // namespace mango