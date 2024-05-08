#pragma once
#include <map>
#include <vector>
#include <volk.h>

namespace mango {
class PhysicalDevice final {
public:
  static std::vector<PhysicalDevice> getPhysicalDevices(VkInstance instance);

  PhysicalDevice() = default;

  PhysicalDevice(const PhysicalDevice &) = delete;
  PhysicalDevice &operator=(const PhysicalDevice &) = delete;

  ~PhysicalDevice() = default;

  uint32_t getGraphicsQueueFamilyIndex() const {
    return graphics_queue_family_index_;
  }

  uint32_t getTransferQueueFamilyIndex() const {
    return transfer_queue_family_index_;
  }

  VkPhysicalDevice getHandle() const { return physical_device_; }

  VkPhysicalDeviceProperties getProperties() const { return properties_; }

  VkPhysicalDeviceFeatures getFeatures() const { return features_; }

  std::vector<VkExtensionProperties> getExtensionProperties() const {
    return extensions_;
  }

  void getExtensionFeatures(void *feature_extension_list) const {
    // Get the extension feature
    VkPhysicalDeviceFeatures2KHR physical_device_features{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR};
    physical_device_features.pNext = feature_extension_list;
    vkGetPhysicalDeviceFeatures2KHR(physical_device_,
                                    &physical_device_features);
  }

private:
  VkPhysicalDevice physical_device_{VK_NULL_HANDLE};
  VkPhysicalDeviceProperties properties_;
  VkPhysicalDeviceFeatures features_;             //!< supported features
  std::vector<VkExtensionProperties> extensions_; //!< supported extensions
  uint32_t graphics_queue_family_index_{0xFFFFFFFF};
  uint32_t transfer_queue_family_index_{0xFFFFFFFF};
  std::vector<VkQueueFamilyProperties> queue_families_;
};
} // namespace mango