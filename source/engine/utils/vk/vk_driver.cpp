
#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <volk.h>

#include <engine/functional/global/engine_context.h>
#include <engine/platform/window.h>
#include <engine/utils/base/error.h>
#include <engine/utils/event/event_system.h>
#include <engine/utils/vk/commands.h>
#include <engine/utils/vk/descriptor_set.h>
#include <engine/utils/vk/framebuffer.h>
#include <engine/utils/vk/physical_device.h>
#include <engine/utils/vk/stage_pool.h>
#include <engine/utils/vk/swapchain.h>
#include <engine/utils/vk/syncs.h>
#include <engine/utils/vk/vk_constants.h>
#include <engine/utils/vk/vk_driver.h>
#include <engine/utils/vk/syncs.h>

namespace mango {

void VkDriver::initInstance() {
  if (VK_SUCCESS != volkInitialize()) {
    throw std::runtime_error("failed to initialize volk!");
  }

  VkApplicationInfo app_info{};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "mango";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "No Engine";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0); // 以上这些意义不大
  app_info.apiVersion = config_->getVersion();

  VkInstanceCreateInfo instance_info{};
  instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_info.pApplicationInfo = &app_info;
  config_->checkAndUpdate(instance_info);

  if (VK_SUCCESS != vkCreateInstance(&instance_info, nullptr, &instance_))
    throw std::runtime_error("failed to create instance");

  volkLoadInstanceOnly(instance_);
}

void VkDriver::initDevice() {
  surface_ = g_engine.getWindow()->createSurface(instance_);
  auto physical_devices = PhysicalDevice::getPhysicalDevices(instance_);
  VkDeviceCreateInfo device_info{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
  const uint32_t physical_device_index =
      config_->checkSelectAndUpdate(physical_devices,
                                                   device_info, surface_);

  if (physical_device_index == -1) {
    throw std::runtime_error("failed to select physical device!");
  }

  auto & ph_device = physical_devices[physical_device_index];
  physical_device_ = ph_device.getHandle();
  auto graphics_queue_family_index = ph_device.getGraphicsQueueFamilyIndex();
  // get minUniformBufferOffsetAlignment
  min_ubo_align_size_ = ph_device.getProperties().limits.minUniformBufferOffsetAlignment;

// for print out memory infos
#ifndef NDEBUG
  VkPhysicalDeviceMemoryProperties memory_properties;
  vkGetPhysicalDeviceMemoryProperties(physical_device_, &memory_properties);
  const char *flag_names[] = {"NONE", "VK_MEMORY_HEAP_DEVICE_LOCAL_BIT",
                              "VK_MEMORY_HEAP_MULTI_INSTANCE_BIT",
                              "VK_MEMORY_HEAP_MULTI_INSTANCE_BIT_KHR"};
  for (uint32_t i = 0; i < memory_properties.memoryHeapCount; ++i) {
    LOGD("heap {0:d} size {1:d}MB flags {2:s}", i,
         memory_properties.memoryHeaps[i].size / 1000000,
         flag_names[static_cast<uint32_t>(
             memory_properties.memoryHeaps[i].flags)]);
  }
  for (uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i) {
    std::stringstream ss;
    if (memory_properties.memoryTypes[i].propertyFlags &
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
      ss << "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ";
    if (memory_properties.memoryTypes[i].propertyFlags &
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
      ss << "VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ";
    if (memory_properties.memoryTypes[i].propertyFlags &
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
      ss << "VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ";
    if (memory_properties.memoryTypes[i].propertyFlags &
        VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
      ss << "VK_MEMORY_PROPERTY_HOST_CACHED_BIT ";
    if (memory_properties.memoryTypes[i].propertyFlags &
        VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
      ss << "VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT ";
    LOGD("heap index {0:d} memory property flags {1:x} {2:s}",
         memory_properties.memoryTypes[i].heapIndex,
         memory_properties.memoryTypes[i].propertyFlags, ss.str());
  }
#endif

  // queue info
  VkDeviceQueueCreateInfo queue_info{};
  float queue_priority[2] = {1.0f, 1.0f};
  queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_info.queueFamilyIndex = graphics_queue_family_index;
  queue_info.queueCount = 2;
  queue_info.pQueuePriorities = queue_priority;

  // logical device
  device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_info.pQueueCreateInfos = &queue_info;
  device_info.queueCreateInfoCount = 1;

  VK_THROW_IF_ERROR(
      vkCreateDevice(physical_device_, &device_info, nullptr, &device_),
      "failed to create vulkan device!");
  volkLoadDevice(device_);
  graphics_cmd_queue_ = new CommandQueue(
      device_, graphics_queue_family_index,
      VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_COMPUTE_BIT,
      VK_TRUE, 0);
  transfer_cmd_queue_ = new CommandQueue(
      device_, graphics_queue_family_index, VK_QUEUE_TRANSFER_BIT, VK_TRUE, 0);
}

void VkDriver::initThreadLocalCommandBufferManagers(const std::initializer_list<uint32_t> & queue_family_indices)
{
  for(auto findex : queue_family_indices)
  {
    thread_local_command_buffer_managers_.emplace_back(shared_from_this(), findex);
  }
}

ThreadLocalCommandBufferManager & VkDriver::getThreadLocalCommandBufferManager()
{
  auto tid = std::this_thread::get_id();
  auto itr = std::find_if(thread_local_command_buffer_managers_.begin(),
               thread_local_command_buffer_managers_.end(),
               [&tid](auto &manager) { return manager.getThreadId() == tid; });
  if (itr == thread_local_command_buffer_managers_.end()) {
    throw std::runtime_error("ThreadLocalCommandBufferManager not found!");
  }
  return *itr;
}

void VkDriver::init(const std::shared_ptr<VkConfig> &config) {
  config_ = config;
  initInstance();
  assert(g_engine.getWindow() != nullptr);
  initDevice();
  initAllocator();

  createSwapchain();
  createFramesData();
  stage_pool_ = new StagePool(shared_from_this());
  #if !NDEBUG
    setupDebugMessenger();
  #endif
}

void VkDriver::createSwapchain() {
  uint32_t width, height;
  auto window = g_engine.getWindow();
  window->getWindowSize(width, height);

  if(swapchain_ == nullptr)
  {
    SwapchainProperties properties{
        .extent = {width, height},
        .surface_format = {.format = VK_FORMAT_B8G8R8A8_SRGB},
    };

    swapchain_ = new Swapchain(surface_, properties);
  } else {
    // recreate swapchain
    // wait idle and clean up render targets/frame buffers swapchain
    waitIdle();
    swapchain_->update(surface_, width, height);
  }

  // event for render system to update
  // recreate framebuffers
  // update camera aspect ratio
  g_engine.getEventSystem()->syncDispatch(
      std::make_shared<RenderCreateSwapchainObjectsEvent>(width, height));
}

void VkDriver::createFramesData() {
  for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
    frames_data_.render_result_available_semaphore[i] =
        std::make_shared<Semaphore>(shared_from_this());
    frames_data_.image_available_semaphore[i] =
        std::make_shared<Semaphore>(shared_from_this());
  }
}

bool VkDriver::waitFrame() {
  auto & mgr = getThreadLocalCommandBufferManager();
  mgr.getCommandBufferAvailableFence()->wait();
  auto result = swapchain_->acquireNextImage(
      frames_data_.image_available_semaphore[cur_frame_index_]->getHandle(),
      VK_NULL_HANDLE, cur_image_index_);  
  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    createSwapchain();
    return false;
  } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw VulkanException(result, "failed to acquire next image");
  }
  mgr.resetCurFrameCommandPool();
  return true;
}

void VkDriver::presentFrame() {
  // present
  VkPresentInfoKHR present_info{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
  VkSwapchainKHR swapchain = swapchain_->getHandle();

  present_info.swapchainCount = 1;
  present_info.pSwapchains = &swapchain;
  present_info.pImageIndices = &cur_image_index_;
  present_info.waitSemaphoreCount = 1;
  auto render_semaphore_handle =
      frames_data_
          .render_result_available_semaphore[cur_frame_index_]->getHandle();
  present_info.pWaitSemaphores = &render_semaphore_handle;

  // Present swapchain image  
  auto result = graphics_cmd_queue_->present(present_info);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
    createSwapchain();
  } else if (result != VK_SUCCESS) {
    throw VulkanException(result, "failed to present image");
  }
  cur_frame_index_ = (cur_frame_index_ + 1) % MAX_FRAMES_IN_FLIGHT;
}

// void VkDriver::createDescriptorPool() {
//   // descriptor pool
//   VkDescriptorPoolSize pool_size[] = {
//       {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
//        MAX_GLOBAL_DESC_SET * CONFIG_UNIFORM_BINDING_COUNT},
//       {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
//        MAX_GLOBAL_DESC_SET * MAX_TEXTURE_NUM_COUNT},
//       {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
//        MAX_GLOBAL_DESC_SET * CONFIG_STORAGE_BINDING_COUNT}};
//   descriptor_pool_ = new DescriptorPool(
//       shared_from_this(), VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
//       pool_size, sizeof(pool_size) / sizeof(pool_size[0]), MAX_GLOBAL_DESC_SET);
// }

bool VkDriver::isDeviceExtensionEnabled(const char *extension_name) {
  const auto &enables = config_->getEnableds();
  for (auto i = 0; i < enables.size(); ++i) {
    auto e = enables[i];
    if (e == VkConfig::EnableState::REQUIRED && strcmp(extension_name, kFeatureExtensionNames[i])==0)
      return true;
  }
  return false;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT messageType,
              const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
              void *pUserData) {
  LOGE("validation layer: {}", pCallbackData->pMessage);
  return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

VkResult vkDestroyDebugReportCallbackEXT(VkInstance instance,
                                         VkDebugUtilsMessengerEXT messager,
                                         const VkAllocationCallbacks *pAllocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr) {
    func(instance, messager, pAllocator);
    return VK_SUCCESS;
  }
  return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void VkDriver::setupDebugMessenger() {

  VkDebugUtilsMessengerCreateInfoEXT createInfo;
  createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  createInfo.pfnUserCallback = debugCallback;

  if (CreateDebugUtilsMessengerEXT(instance_, &createInfo, nullptr,
                                   &debug_messenger_) != VK_SUCCESS) {
    throw std::runtime_error("failed to set up debug messenger!");
  }
}

void VkDriver::checkSwapchainAbility() {
  VkSurfaceFormatKHR surface_format{VK_FORMAT_B8G8R8A8_SRGB,
                                    VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
  uint32_t format_count = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface_,
                                       &format_count, nullptr);
  std::vector<VkSurfaceFormatKHR> surface_formats(format_count);
  vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface_,
                                       &format_count, surface_formats.data());
  auto format_itr =
      std::find_if(surface_formats.begin(), surface_formats.end(),
                   [&surface_format](const VkSurfaceFormatKHR sf) {
                     return sf.format == surface_format.format &&
                            sf.colorSpace == surface_format.colorSpace;
                   });
  if (format_itr == surface_formats.end()) {
    throw std::runtime_error(
        "B8G8R8_SNORM format, COLOR_SPACE_SRGB_NONLINEAR_KHR is not "
        "supported!");
  }

  uint32_t present_mode_count = 0;
  vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device_, surface_,
                                            &present_mode_count, nullptr);
  std::vector<VkPresentModeKHR> present_modes(present_mode_count);
  vkGetPhysicalDeviceSurfacePresentModesKHR(
      physical_device_, surface_, &present_mode_count, present_modes.data());
  auto present_itr =
      std::find_if(present_modes.begin(), present_modes.end(),
                   [](const VkPresentModeKHR present_mode) {
                     return present_mode == VK_PRESENT_MODE_FIFO_KHR;
                   });
  if (present_itr == present_modes.end()) {
    throw std::runtime_error("FIFO present mode is not supported!");
  }
}

void VkDriver::initAllocator() {
  VmaVulkanFunctions vma_vulkan_func{};
  vma_vulkan_func.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
  vma_vulkan_func.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
  vma_vulkan_func.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
  vma_vulkan_func.vkAllocateMemory = vkAllocateMemory;
  vma_vulkan_func.vkBindBufferMemory = vkBindBufferMemory;
  vma_vulkan_func.vkBindBufferMemory2KHR = (vkBindBufferMemory2 != nullptr)
                                               ? vkBindBufferMemory2
                                               : vkBindBufferMemory2KHR;
  vma_vulkan_func.vkBindImageMemory = vkBindImageMemory;
  vma_vulkan_func.vkBindImageMemory2KHR = (vkBindImageMemory2 != nullptr)
                                              ? vkBindImageMemory2
                                              : vkBindImageMemory2KHR;
  vma_vulkan_func.vkCreateBuffer = vkCreateBuffer;
  vma_vulkan_func.vkCreateImage = vkCreateImage;
  vma_vulkan_func.vkDestroyBuffer = vkDestroyBuffer;
  vma_vulkan_func.vkDestroyImage = vkDestroyImage;
  vma_vulkan_func.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
  vma_vulkan_func.vkFreeMemory = vkFreeMemory;
  vma_vulkan_func.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
  vma_vulkan_func.vkGetBufferMemoryRequirements2KHR =
      vkGetBufferMemoryRequirements2KHR;
  vma_vulkan_func.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
  vma_vulkan_func.vkGetImageMemoryRequirements2KHR =
      vkGetImageMemoryRequirements2KHR;
  vma_vulkan_func.vkGetPhysicalDeviceMemoryProperties =
      vkGetPhysicalDeviceMemoryProperties;
  vma_vulkan_func.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
  vma_vulkan_func.vkGetPhysicalDeviceMemoryProperties2KHR =
      (vkGetPhysicalDeviceMemoryProperties2 != nullptr)
          ? vkGetPhysicalDeviceMemoryProperties2
          : vkGetPhysicalDeviceMemoryProperties2KHR;
  vma_vulkan_func.vkInvalidateMappedMemoryRanges =
      vkInvalidateMappedMemoryRanges;
  vma_vulkan_func.vkMapMemory = vkMapMemory;
  vma_vulkan_func.vkUnmapMemory = vkUnmapMemory;
  vma_vulkan_func.vkCmdCopyBuffer = vkCmdCopyBuffer;

  if (config_->getVersion() >= VK_MAKE_VERSION(1, 3, 0)) {
    vma_vulkan_func.vkGetDeviceBufferMemoryRequirements =
        vkGetDeviceBufferMemoryRequirements;
    vma_vulkan_func.vkGetDeviceImageMemoryRequirements =
        vkGetDeviceImageMemoryRequirements;
  }

  VmaAllocatorCreateInfo allocator_info{};
  allocator_info.physicalDevice = physical_device_;
  allocator_info.device = device_;
  allocator_info.instance = instance_;
  allocator_info.vulkanApiVersion = config_->getVersion();
  allocator_info.pVulkanFunctions = &vma_vulkan_func;

  bool can_get_memory_requirements =
      isDeviceExtensionEnabled(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
  bool has_dedicated_allocation =
      isDeviceExtensionEnabled(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
  bool can_get_buffer_device_address =
      isDeviceExtensionEnabled(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);

  if (can_get_memory_requirements && has_dedicated_allocation) {
    allocator_info.flags |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;
    vma_vulkan_func.vkGetBufferMemoryRequirements2KHR =
        vkGetBufferMemoryRequirements2KHR;
    vma_vulkan_func.vkGetImageMemoryRequirements2KHR =
        vkGetImageMemoryRequirements2KHR;
  }

  if (can_get_buffer_device_address) {
    allocator_info.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
  }

  auto result = vmaCreateAllocator(&allocator_info, &allocator_);
  if (result != VK_SUCCESS) {
    throw VulkanException(result, "failed to create vma allocator");
  }
}

void VkDriver::destroy() {
  thread_local_command_buffer_managers_.clear();
  frames_data_.destroy();
  delete swapchain_;
  delete descriptor_pool_;
  delete stage_pool_;
  delete graphics_cmd_queue_;
  if (allocator_) {
    // VmaTotalStatistics stats;
    // vmaCalculateStatistics(allocator_, &stats);
    // LOGI("Total device memory leaked: {} bytes.", stats.total.);
    vmaDestroyAllocator(allocator_);
  }

  vkDestroyDevice(device_, nullptr);
  vkDestroySurfaceKHR(instance_, surface_, nullptr);
  if (debug_messenger_ != VK_NULL_HANDLE) {
    vkDestroyDebugReportCallbackEXT(instance_, debug_messenger_, nullptr);
  }
  vkDestroyInstance(instance_, nullptr);
}
} // namespace mango
