#pragma once

#include <volk.h>
#include <string>

namespace mango
{
    struct ImageMemoryBarrier
    {
        VkImageLayout old_layout;
        VkImageLayout new_layout;
        VkAccessFlags src_access_mask;
        VkAccessFlags dst_access_mask;
        VkPipelineStageFlags src_stage_mask;
        VkPipelineStageFlags dst_stage_mask;
        uint32_t src_queue_family_index;
        uint32_t dst_queue_family_index;
    };
} // namespace mango