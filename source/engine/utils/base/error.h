#pragma once

#include <engine/utils/base/macro.h>
#include <engine/utils/base/strings.h>
#include <exception>

// /// @brief Helper macro to test the result of Vulkan calls which can return
// an
// /// error.
// #define VK_CHECK(x) \
//   do { \
//     VkResult err = x; \
//     if (err) { \
//       LOGE("Detected Vulkan error: {}", vkb::to_string(err)); \
//       abort(); \
//     } \
//   } while (0)

// #define ASSERT_VK_HANDLE(handle)                                               \
//   do {                                                                         \
//     if ((handle) == VK_NULL_HANDLE) {                                          \
//       LOGE("Handle is NULL");                                                  \
//       abort();                                                                 \
//     }                                                                          \
//   } while (0)

namespace mango {

class VulkanException : public std::runtime_error {
public:
  /**
   * @brief Vulkan exception constructor
   */
  VulkanException(VkResult result, const std::string &msg = "Vulkan error")
      : std::runtime_error(msg), result(result) {}

  /**
   * @brief Returns the Vulkan error code as string
   * @return String message of exception
   */
  const char *what() const noexcept override { return error_message.c_str(); }

  VkResult result;

private:
  std::string error_message;
};
} // namespace mango

class VulkanUseException : public std::logic_error {
public:
  /**
   * @brief Vulkan exception constructor
   */
  VulkanUseException(const std::string &msg = "Vulkan logic error")
      : std::logic_error(msg) {}

  /**
   * @brief Returns the Vulkan error code as string
   * @return String message of exception
   */
  const char *what() const noexcept override { return error_message.c_str(); }

private:
  std::string error_message;
};

#define VK_THROW_IF_ERROR(result, msg)                                         \
  if (result != VK_SUCCESS) {                                                  \
    throw mango::VulkanException(result, msg);                                 \
  }
