#pragma once
#include <engine/functional/global/engine_context.h>
#include <engine/utils/base/strings.h>

#define LOGI(...)
#define LOGW(...)
#define LOGE(...)
#define LOGF(...)
#define LOGD(...)

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

// #define TO_ABSOLUTE(path) g_engine.getFileSystem()->absolute(path)