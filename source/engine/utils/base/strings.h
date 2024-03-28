#pragma once

#include <string>
#include <volk.h>

namespace vkb {
/**
 * @brief Helper function to convert a VkResult enum to a string
 * @param result Vulkan result to convert.
 * @return The string to return.
 */
const std::string to_string(VkResult result);
} // namespace vkb
