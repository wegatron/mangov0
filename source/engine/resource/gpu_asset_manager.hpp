#pragma once

#include <engine/functional/global/engine_context.h>
#include <map>
#include <memory>
#include <stdexcept>

namespace mango {

class ImageView; // in visualstudio stract and class use different namemangling
                 // rules
class CommandBuffer;

struct Asset {
  std::shared_ptr<void> data_ptr;
  mutable uint64_t last_accessed;
};

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

/**
 * \brief GPUAssertManager is used to manage the GPU assert.
 * The assert is load from file, and will not change.
 */
class GPUAssetManager final {
public:
  GPUAssetManager() = default;

  ~GPUAssetManager() = default;

  template <typename T>
  [[nondiscard]] std::shared_ptr<T>
  request(const std::string &path,
          const std::shared_ptr<CommandBuffer> &cmd_buf) {
    auto itr = assets_.find(path);
    if (itr != assets_.end()) {
      return std::static_pointer_cast<T>(itr->second.data_ptr);
    }
    if (cmd_buf == nullptr) {
      auto driver = g_engine.getDriver();
      auto ccmd_buf = driver->requestCommandBuffer(
          VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY);
      auto ret = load<T>(path, ccmd_buf);
      driver->getGraphicsQueue()->submit(ccmd_buf);
      driver->getGraphicsQueue()->waitIdle();
      assets_.emplace(path, Asset{ret, current_frame_});
      return ret;
    }
    auto ret = load<T>(path, cmd_buf);
    assets_.emplace(path, Asset{ret, current_frame_});
    return ret;
  }

  template <typename T>
  [[nondiscard]] std::shared_ptr<T>
  request(const uint8_t *data, const size_t size,
          const std::shared_ptr<CommandBuffer> &cmd_buf) {
    if (cmd_buf == nullptr) {
      auto driver = g_engine.getDriver();
      auto ccmd_buf = driver->requestCommandBuffer(
          VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY);
      auto ret = load<T>(data, size, ccmd_buf);
      driver->getGraphicsQueue()->submit(ccmd_buf);
      driver->getGraphicsQueue()->waitIdle();
      return ret;
    }
    auto ret = load<T>(data, size, cmd_buf);
    return ret;
  }

  template <typename T>
  std::shared_ptr<T> request(const float *data, const uint32_t width,
                             const uint32_t height, const uint32_t channel,
                             const std::shared_ptr<CommandBuffer> &cmd_buf) {
    if (cmd_buf == nullptr) {
      auto driver = g_engine.getDriver();
      auto ccmd_buf = driver->requestCommandBuffer(
          VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY);
      auto ret = load<T>(data, width, height, channel, ccmd_buf);
      driver->getGraphicsQueue()->submit(ccmd_buf);
      driver->getGraphicsQueue()->waitIdle();
      return ret;
    }
    auto ret = load<T>(data, width, height, channel, cmd_buf);
    return ret;
  }

  void gc();

  void reset();

private:
  std::map<std::string, Asset> assets_;
  uint64_t current_frame_{0};
};
} // namespace mango