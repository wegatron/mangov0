﻿#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <vector>
#include <volk.h>

#include <engine/utils/vk/physical_device.h>

#ifdef OPTIONAL
#undef OPTIONAL
#endif

namespace mango {
constexpr char const * kFeatureExtensionNames[] = {
    "LAYER_BEGIN_PIVOT",           // 0
    "VK_LAYER_KHRONOS_validation", // 1
    "LAYER_END_PIVOT",             // 2

    "INSTANCE_EXTENSION_BEGIN_PIVOT",                       // 3
    "GLFW_EXTENSION",                                       // 4
    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, // 5
    VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME,            // 6
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,                      // 7
    "INSTANCE_EXTENSION_END_PIVOT",                         // 8

    "DEVICE_EXTENSION_BEGIN_PIVOT",                       // 9
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,                      // 10
    VK_KHR_UNIFORM_BUFFER_STANDARD_LAYOUT_EXTENSION_NAME, // 11
    VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME,       // 12
    VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,            // 13
    VK_EXT_ROBUSTNESS_2_EXTENSION_NAME,                   // 14
    VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,      // 15
    VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,           // 16
    VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,          // 17
    VK_KHR_DEVICE_GROUP_EXTENSION_NAME,                   // 18
    "DEVICE_EXTENSION_END_PIVOT",                         // 19
};
class VkConfig {
public:
  enum class EnableState : uint8_t { DISABLED = 0, OPTIONAL, REQUIRED };

  enum class FeatureExtension : uint32_t {
    //// instance layers
    LAYER_BEGIN_PIVOT = 0,
    KHR_VALIDATION_LAYER = 1,
    LAYER_END_PIVOT = 2,

    //// instance extension
    INSTANCE_EXTENSION_BEGIN_PIVOT = 3,
    GLFW_EXTENSION = 4,
    INSTANCE_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2 = 5,
    KHR_DEVICE_GROUP_CREATION = 6,
    EXT_DEBUG_UTILS = 7,
    INSTANCE_EXTENSION_END_PIVOT = 8,

    //// device extension
    DEVICE_EXTENSION_BEGIN_PIVOT = 9,
    KHR_SWAPCHAIN = 10,
    KHR_UNIFORM_BUFFER_STANDARD_LAYOUT = 11,
    VK_KHR_SHADER_NON_SEMANTIC_INFO = 12,
    DESCRIPTOR_INDEX = 13,
    VK_EXT_ROBUSTNESS_2 = 14,

    // VMA support these extensions
    KHR_GET_MEMORY_REQUIREMENTS_2 = 15,
    KHR_DEDICATED_ALLOCATION = 16,
    KHR_BUFFER_DEVICE_ADDRESS = 17,
    KHR_DEVICE_GROUP = 18,
    DEVICE_EXTENSION_END_PIVOT = 19,

    //// Device features
    MAX_FEATURE_EXTENSION_COUNT
  };

  VkConfig()
      : enableds_(static_cast<uint32_t>(
                      FeatureExtension::MAX_FEATURE_EXTENSION_COUNT),
                  EnableState::DISABLED) {}

  virtual ~VkConfig() = default;

  void setFeatureEnabled(FeatureExtension feature, EnableState enable_state) {
    enableds_[static_cast<uint32_t>(feature)] = enable_state;
  }

  EnableState isFeatureEnabled(FeatureExtension feature) const {
    return enableds_[static_cast<uint32_t>(feature)];
  }

  /**
   * \brief Check and update the instance create info, for enable layers and
   * extensions.
   */
  void checkAndUpdate(VkInstanceCreateInfo &create_info) {
    checkAndUpdateLayers(create_info);
    checkAndUpdateExtensions(create_info);
  }

  /**
   * \brief check and select the best physical device, for enable features and
   * extensions.
   *
   * \return the index of the best physical device.
   */
  virtual uint32_t
  checkSelectAndUpdate(const std::vector<PhysicalDevice> &physical_devices,
                       VkDeviceCreateInfo &create_info,
                       VkSurfaceKHR surface) = 0;
  void setDeviceType(VkPhysicalDeviceType device_type) {
    device_type_ = device_type;
  }

  uint32_t getVersion() const noexcept { return version_; }

  const std::vector<EnableState> &getEnableds() const noexcept {
    return enableds_;
  }

protected:
  virtual void checkAndUpdateLayers(VkInstanceCreateInfo &create_info) = 0;
  virtual void checkAndUpdateExtensions(VkInstanceCreateInfo &create_info) = 0;

  VkPhysicalDeviceType device_type_{VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU};
  uint32_t version_{VK_API_VERSION_1_0};

  std::vector<EnableState> enableds_;
  std::vector<std::pair<const char *, EnableState>> request_layers_;
  std::vector<std::pair<const char *, EnableState>> request_device_extensions_;

  std::vector<const char *> enabled_layers_;
  std::vector<const char *> enabled_instance_extensions_;
  std::vector<const char *> enabled_device_extensions_;
  std::map<VkStructureType, std::shared_ptr<void>> extension_features_;
  void *extension_features_list_{nullptr}; //!< wrapper not own memory
  VkPhysicalDeviceFeatures device_features_{VK_FALSE};
};

class Vk13Config : public VkConfig {
public:
  Vk13Config() : VkConfig() {
    version_ = VK_API_VERSION_1_3;
#if !NDEBUG
    // for validation and debug message
    enableds_[static_cast<uint32_t>(
        VkConfig::FeatureExtension::KHR_VALIDATION_LAYER)] =
        VkConfig::EnableState::REQUIRED;
    enableds_[static_cast<uint32_t>(FeatureExtension::EXT_DEBUG_UTILS)] =
        EnableState::REQUIRED;
#endif

    // for vma
    enableds_[static_cast<uint32_t>(
        FeatureExtension::KHR_GET_MEMORY_REQUIREMENTS_2)] =
        EnableState::REQUIRED;
    enableds_[static_cast<uint32_t>(
        FeatureExtension::KHR_DEDICATED_ALLOCATION)] = EnableState::REQUIRED;
    enableds_[static_cast<uint32_t>(
        FeatureExtension::KHR_BUFFER_DEVICE_ADDRESS)] = EnableState::REQUIRED;
    enableds_[static_cast<uint32_t>(
        FeatureExtension::KHR_DEVICE_GROUP_CREATION)] = EnableState::REQUIRED;
    enableds_[static_cast<uint32_t>(FeatureExtension::KHR_DEVICE_GROUP)] =
        EnableState::REQUIRED;
    enableds_[static_cast<uint32_t>(FeatureExtension::DESCRIPTOR_INDEX)] =
        EnableState::REQUIRED;

    // enableds_[static_cast<uint32_t>(FeatureExtension::KHR_UNIFORM_BUFFER_STANDARD_LAYOUT)]
    // = EnableState::REQUIRED;
    enableds_[static_cast<uint32_t>(
        FeatureExtension::VK_KHR_SHADER_NON_SEMANTIC_INFO)] =
        EnableState::REQUIRED;
    enableds_[static_cast<uint32_t>(FeatureExtension::VK_EXT_ROBUSTNESS_2)] =
        EnableState::REQUIRED;
  }

  ~Vk13Config() override = default;

  uint32_t
  checkSelectAndUpdate(const std::vector<PhysicalDevice> &physical_devices,
                       VkDeviceCreateInfo &create_info,
                       VkSurfaceKHR surface) override;

protected:
  void checkAndUpdateLayers(VkInstanceCreateInfo &create_info) override;
  void checkAndUpdateExtensions(VkInstanceCreateInfo &create_info) override;
};
} // namespace mango