#include <GLFW/glfw3.h>
#include <engine/utils/base/error.h>
#include <engine/utils/vk/vk_config.h>
#include <string.h>

namespace mango {

uint32_t Vk13Config::checkSelectAndUpdate(
    const std::vector<PhysicalDevice> &physical_devices,
    VkDeviceCreateInfo &create_info, VkSurfaceKHR surface) {
  for (auto i = static_cast<uint32_t>(
                    FeatureExtension::DEVICE_EXTENSION_BEGIN_PIVOT) +
                1;
       i < static_cast<uint32_t>(FeatureExtension::DEVICE_EXTENSION_END_PIVOT);
       ++i) {
    if (enableds_[i] != EnableState::DISABLED) {
      request_device_extensions_.emplace_back(kFeatureExtensionNames[i],
                                              enableds_[i]);
    }
  }

  if (enableds_[static_cast<uint32_t>(
          FeatureExtension::KHR_BUFFER_DEVICE_ADDRESS)] !=
      EnableState::DISABLED) {
    auto ext_feature =
        std::make_shared<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>();
    extension_features_.emplace(
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_KHR,
        ext_feature);
    ext_feature->sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_KHR;
    ext_feature->pNext = extension_features_list_;
    extension_features_list_ = ext_feature.get();
  }

  if (enableds_[static_cast<uint32_t>(FeatureExtension::DESCRIPTOR_INDEX)] !=
      EnableState::DISABLED) {
    auto ext_feature =
        std::make_shared<VkPhysicalDeviceDescriptorIndexingFeatures>();
    extension_features_.emplace(
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
        ext_feature);
    ext_feature->sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
    ext_feature->descriptorBindingPartiallyBound = VK_TRUE;
    ext_feature->pNext = extension_features_list_;
    extension_features_list_ = ext_feature.get();
  }

  if (enableds_[static_cast<uint32_t>(FeatureExtension::VK_EXT_ROBUSTNESS_2)] !=
      EnableState::DISABLED) {
    auto ext_feature =
        std::make_shared<VkPhysicalDeviceRobustness2FeaturesEXT>();
    extension_features_.emplace(
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT,
        ext_feature);
    ext_feature->sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT;
    ext_feature->nullDescriptor = VK_TRUE;
    ext_feature->pNext = extension_features_list_;
    extension_features_list_ = ext_feature.get();
    
    // though robustBufferAccess2 is false, but still enabled, so set  robustBufferAccess here
    device_features_.robustBufferAccess = VK_TRUE;
  }

  uint32_t selected_physical_device_index = -1;
  enabled_device_extensions_.reserve(request_device_extensions_.size());
  for (uint32_t device_index = 0; device_index < physical_devices.size();
       ++device_index) {
    enabled_device_extensions_.clear();
    auto &pd = physical_devices[device_index];
    auto handle = pd.getHandle();
    uint32_t graphics_queue_family_index = pd.getGraphicsQueueFamilyIndex();
    if (graphics_queue_family_index == 0XFFFFFFFF)
      continue;
    VkBool32 surface_support = (surface == nullptr);
    if (!surface_support)
      vkGetPhysicalDeviceSurfaceSupportKHR(handle, graphics_queue_family_index,
                                           surface, &surface_support);
    if (!surface_support || (pd.getProperties().deviceType != device_type_))
      continue;

    const auto &device_extensions = pd.getExtensionProperties();
#ifndef NDEBUG
    LOGD("device name: {}", pd.getProperties().deviceName);
    for (const auto &ext : device_extensions)
      LOGD("device extension: {}", ext.extensionName);

    LOGD("---------requested extension begin-------");
    for (const auto &ext : request_device_extensions_) {
      LOGD("{}", ext.first);
    }
    LOGD("---------requested extension end---------");
#endif
    bool extension_support = true;
    for (const auto &req_ext : request_device_extensions_) {
      bool is_find = false;
      for (const auto &ext : device_extensions) {
        if (strcmp(ext.extensionName, req_ext.first) == 0) {
          is_find = true;
          break;
        }
      }
      if (is_find) {
        enabled_device_extensions_.emplace_back(req_ext.first);
        continue;
      }
      if (req_ext.second == EnableState::REQUIRED) {
        extension_support = false;
        break;
      }
    }

    if (!extension_support)
      continue;

    // feature extension support
    if (extension_features_list_ != nullptr) {
      pd.getExtensionFeatures(extension_features_list_);
    }
    selected_physical_device_index = device_index;
    break;
  }

  if (selected_physical_device_index == -1)
    return selected_physical_device_index;

  // TODO setup device features

  // update device create info
  create_info.pEnabledFeatures = &device_features_;
  create_info.enabledExtensionCount = enabled_device_extensions_.size();
  create_info.ppEnabledExtensionNames = enabled_device_extensions_.data();
  create_info.pNext = extension_features_list_;
  return selected_physical_device_index;
}

void Vk13Config::checkAndUpdateLayers(VkInstanceCreateInfo &create_info) {
  for (auto i = static_cast<uint32_t>(FeatureExtension::LAYER_BEGIN_PIVOT) + 1;
       i < static_cast<uint32_t>(FeatureExtension::LAYER_END_PIVOT); ++i) {
    if (enableds_[i] != EnableState::DISABLED) {
      request_layers_.emplace_back(kFeatureExtensionNames[i], enableds_[i]);
    }
  }

  uint32_t layer_count = 0;
  vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
  std::vector<VkLayerProperties> available_layers(layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());
  enabled_layers_.reserve(request_layers_.size());
  for (const auto layer : request_layers_) {
    bool is_find = false;
    for (const auto &l : available_layers) {
      if (strcmp(l.layerName, layer.first) == 0) {
        is_find = true;
        enabled_layers_.emplace_back(layer.first);
        break;
      }
    }

    if (!is_find) {
      std::string info = std::string("validation layer ") +
                         std::string(layer.first) +
                         " requested, but not available!";
      if (layer.second == EnableState::REQUIRED) {
        throw VulkanException(VK_RESULT_MAX_ENUM, info);
      } else {
        LOGI(info);
        continue;
      }
    }
  }

  create_info.enabledLayerCount = enabled_layers_.size();
  create_info.ppEnabledLayerNames = enabled_layers_.data();
}

void Vk13Config::checkAndUpdateExtensions(VkInstanceCreateInfo &create_info) {
  enabled_instance_extensions_.clear();
  uint32_t extension_count = 0;
  for (auto i = static_cast<uint32_t>(
                    FeatureExtension::INSTANCE_EXTENSION_BEGIN_PIVOT) +
                1;
       i <
       static_cast<uint32_t>(FeatureExtension::INSTANCE_EXTENSION_END_PIVOT);
       ++i) {
    if (enableds_[i] != EnableState::DISABLED) {
      if (static_cast<uint32_t>(FeatureExtension::GLFW_EXTENSION) == i) {
        uint32_t glfw_extension_count = 0;
        const char **glfw_extensions =
            glfwGetRequiredInstanceExtensions(&glfw_extension_count);
        for (uint32_t j = 0; j < glfw_extension_count; ++j) {
          enabled_instance_extensions_.emplace_back(glfw_extensions[j]);
        }
        extension_count += glfw_extension_count;
      } else {
        enabled_instance_extensions_.emplace_back(kFeatureExtensionNames[i]);
        ++extension_count;
      }
    }
  }

  create_info.enabledExtensionCount = extension_count;
  create_info.ppEnabledExtensionNames = enabled_instance_extensions_.data();
}

} // namespace mango