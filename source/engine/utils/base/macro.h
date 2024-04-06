#pragma once

#include <engine/functional/global/engine_context.h>
#include <engine/utils/base/strings.h>
#include <engine/utils/log/log_system.h>
#include <engine/platform/file_system.h>

#define LOGI(...)                                                              \
  SPDLOG_LOGGER_INFO(g_engine.getLogSystem()->getLogger(), __VA_ARGS__)
#define LOGW(...)                                                              \
  SPDLOG_LOGGER_WARN(g_engine.getLogSystem()->getLogger(), __VA_ARGS__)
#define LOGE(...)                                                              \
  SPDLOG_LOGGER_ERROR(g_engine.getLogSystem()->getLogger(), __VA_ARGS__)
#define LOGF(...)                                                              \
  SPDLOG_LOGGER_CRITICAL(g_engine.getLogSystem()->getLogger(), __VA_ARGS__)
#define LOGD(...)                                                              \
  SPDLOG_LOGGER_DEBUG(g_engine.getLogSystem()->getLogger(), __VA_ARGS__)

#define VK_ASSERT(err, msg)                                                    \
  do {                                                                         \
    if (err) {                                                                 \
      LOGE("{} Vulkan error: {}", msg, vkb::to_string(err));                   \
      abort();                                                                 \
    }                                                                          \
  } while (0)

#define MANGO_ASSERT(x, msg)                                                   \
  do {                                                                         \
    if (!(x)) {                                                                \
      LOGE("Assertion failed: {}", msg);                                       \
      abort();                                                                 \
    }                                                                          \
  } while (0)

#define TO_ABSOLUTE(path) g_engine.getFileSystem()->absolute(path)