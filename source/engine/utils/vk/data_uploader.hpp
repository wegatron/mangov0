#pragma once
#include <memory>
#include <string>

namespace mango {
class CommandBuffer;
class ImageView;

std::shared_ptr<ImageView>
uploadImage(const uint8_t *data, const uint32_t width, const uint32_t height,
            const uint32_t mipmap_level, const uint32_t layers,
            const VkFormat format,
            const std::shared_ptr<CommandBuffer> &cmd_buf);

std::shared_ptr<ImageView>
uploadImage(const float *data, const uint32_t width, const uint32_t height,
            const uint32_t mipmap_level, const uint32_t layers, VkFormat format,
            const std::shared_ptr<CommandBuffer> &cmd_buf);
} // namespace mango