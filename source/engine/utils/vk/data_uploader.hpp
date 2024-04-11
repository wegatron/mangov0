#pragma once
#include <memory>
#include <string>

namespace mango {
class CommandBuffer;
class ImageView;

template <typename T>
std::shared_ptr<T> upload(const uint8_t *data, const size_t size,
                          const std::shared_ptr<CommandBuffer> &cmd_buf) {
  throw std::logic_error("load type unsupported!");
}

template <typename T>
std::shared_ptr<T> upload(const float *data, const uint32_t width,
                          const uint32_t height, const uint32_t channel,
                          const std::shared_ptr<CommandBuffer> &cmd_buf) {
  throw std::logic_error("load type unsupported!");
}

template <>
std::shared_ptr<ImageView>
upload(const uint8_t *data, const size_t size,
       const std::shared_ptr<CommandBuffer> &cmd_buf);

template <>
std::shared_ptr<ImageView>
upload(const float *data, const uint32_t width, const uint32_t height,
       const uint32_t channel, const std::shared_ptr<CommandBuffer> &cmd_buf);
} // namespace mango