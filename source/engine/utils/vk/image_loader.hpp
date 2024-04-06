#pragma once
#include <memory>
#include <string>

namespace mango {
class CommandBuffer;
class ImageView;

template <typename T>
std::shared_ptr<T> load(const std::string &path,
                        const std::shared_ptr<CommandBuffer> &cmd_buf) {
  throw std::logic_error("load type unsupported!");
}

template <typename T>
std::shared_ptr<T> load(const uint8_t *data, const size_t size,
                        const std::shared_ptr<CommandBuffer> &cmd_buf) {
  throw std::logic_error("load type unsupported!");
}

template <typename T>
std::shared_ptr<T> load(const float *data, const uint32_t width,
                        const uint32_t height, const uint32_t channel,
                        const std::shared_ptr<CommandBuffer> &cmd_buf) {
  throw std::logic_error("load type unsupported!");
}

template <>
std::shared_ptr<ImageView> load(const std::string &path,
                                const std::shared_ptr<CommandBuffer> &cmd_buf);

template <>
std::shared_ptr<ImageView> load(const uint8_t *data, const size_t size,
                                const std::shared_ptr<CommandBuffer> &cmd_buf);

template <>
std::shared_ptr<ImageView> load(const float *data, const uint32_t width,
                                const uint32_t height, const uint32_t channel,
                                const std::shared_ptr<CommandBuffer> &cmd_buf);
} // namespace mango