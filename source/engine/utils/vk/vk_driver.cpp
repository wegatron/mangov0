
#include <algorithm>
#include <cassert>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <volk.h>

#include <engine/functional/global/engine_context.h>
#include <engine/platform/window.h>
#include <engine/utils/base/error.h>
#include <engine/utils/vk/commands.h>
#include <engine/utils/vk/descriptor_set.h>
#include <engine/utils/vk/framebuffer.h>
#include <engine/utils/vk/physical_device.h>
#include <engine/utils/vk/resource_cache.h>
#include <engine/utils/vk/stage_pool.h>
#include <engine/utils/vk/swapchain.h>
#include <engine/utils/vk/syncs.h>
#include <engine/utils/vk/vk_constants.h>
#include <engine/utils/vk/vk_driver.h>

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
  app_info.apiVersion = g_engine.getVkConfig()->getVersion();

  VkInstanceCreateInfo instance_info{};
  instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_info.pApplicationInfo = &app_info;
  g_engine.getVkConfig()->checkAndUpdate(instance_info);

  if (VK_SUCCESS != vkCreateInstance(&instance_info, nullptr, &instance_))
    throw std::runtime_error("failed to create instance");

  volkLoadInstanceOnly(instance_);
}

void VkDriver::initDevice() {
  surface_ = g_engine.getWindow()->createSurface(instance_);
  auto physical_devices = PhysicalDevice::getPhysicalDevices(instance_);
  VkDeviceCreateInfo device_info{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
  const uint32_t physical_device_index =
      g_engine.getVkConfig()->checkSelectAndUpdate(physical_devices,
                                                   device_info, surface_);

  if (physical_device_index == -1) {
    throw std::runtime_error("failed to select physical device!");
  }

  physical_device_ = physical_devices[physical_device_index].getHandle();
  auto graphics_queue_family_index =
      physical_devices[physical_device_index].getGraphicsQueueFamilyIndex();

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
  float queue_priority = 1.0f;
  queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_info.queueFamilyIndex = graphics_queue_family_index;
  queue_info.queueCount = 1;
  queue_info.pQueuePriorities = &queue_priority;

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
}

void VkDriver::init() {
  initInstance();
  assert(g_engine.getWindow() != nullptr);
  initDevice();
  initAllocator();

  createSwapchain();
  createFramesData();
  createDescriptorPool();
  stage_pool_ = new StagePool(shared_from_this());
}

uint32_t VkDriver::getSwapchainImageCount() const {
  assert(swapchain_ != nullptr);
  return swapchain_->getImageCount();
}

VkFormat VkDriver::getSwapchainImageFormat() const {
  assert(swapchain_ != nullptr);
  return swapchain_->getImageFormat();
}

void VkDriver::createSwapchain() {
  uint32_t width, height;
  auto window = g_engine.getWindow();
  window->getWindowSize(width, height);

  SwapchainProperties properties{
      .extent = {width, height},
      .surface_format = {.format = VK_FORMAT_B8G8R8A8_SRGB},
  };
  if (swapchain_ == nullptr)
    swapchain_ = new Swapchain(surface_, properties);
  else {
    swapchain_->initSwapchain(surface_, properties);
  }

  // create render targets
  const auto img_cnt = swapchain_->getImageCount();
  render_targets_.resize(img_cnt);
  auto driver = g_engine.getDriver();
  for (uint32_t i = 0; i < img_cnt; ++i) {
    // auto depth_image = std::make_shared<Image>(
    //     driver, 0,
    //     ds_format_, extent, VK_SAMPLE_COUNT_1_BIT,
    //     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
    //     VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    // auto depth_img_view = std::make_shared<ImageView>(
    //     depth_image, VK_IMAGE_VIEW_TYPE_2D, ds_format_,
    //     VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 0, 1, 1);
    render_targets_[i] = std::make_shared<RenderTarget>(
        std::initializer_list<std::shared_ptr<ImageView>>{
            swapchain_->getImageView(i)},
        std::initializer_list<VkFormat>{swapchain_->getImageFormat()},
        VK_FORMAT_UNDEFINED, width, height, 1u);
  }

  // TODO event for UIPass to create frame buffer
}

std::shared_ptr<class CommandBuffer>
VkDriver::requestCommandBuffer(VkCommandBufferLevel level) {
  auto cmd_pool = frames_data_[cur_frame_index_].command_pool;
  return cmd_pool->requestCommandBuffer(level);
}

void VkDriver::createFramesData() {
  auto n = swapchain_->getImageCount();
  frames_data_.resize(n);
  for (auto i = 0; i < n; ++i) {
    frames_data_[i].render_fence =
        std::make_shared<Fence>(shared_from_this(), true);
    frames_data_[i].render_semaphore =
        std::make_shared<Semaphore>(shared_from_this());
    frames_data_[i].present_semaphore =
        std::make_shared<Semaphore>(shared_from_this());
    frames_data_[i].command_pool = std::make_shared<CommandPool>(
        shared_from_this(), graphics_cmd_queue_->getFamilyIndex(),
        CommandPool::CmbResetMode::ResetIndividually);
  }
}

void VkDriver::waitFrame() {
  cur_image_index_ = swapchain_->acquireNextImage(
      frames_data_[cur_frame_index_].present_semaphore->getHandle(),
      frames_data_[cur_frame_index_].render_fence->getHandle());
  frames_data_[cur_frame_index_]
      .render_fence->wait(); // wait for cmdbuffer is free
  frames_data_[cur_frame_index_].render_fence->reset(); // reset to unsignaled
  frames_data_[cur_frame_index_].command_pool->reset();
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
      frames_data_[cur_frame_index_].render_semaphore->getHandle();
  present_info.pWaitSemaphores = &render_semaphore_handle;

  // Present swapchain image
  graphics_cmd_queue_->present(present_info);
  cur_frame_index_ = (cur_frame_index_ + 1) % swapchain_->getImageCount();
}

void VkDriver::createDescriptorPool() {
  // descriptor pool
  VkDescriptorPoolSize pool_size[] = {
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
       MAX_GLOBAL_DESC_SET * CONFIG_UNIFORM_BINDING_COUNT},
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
       MAX_GLOBAL_DESC_SET * MAX_TEXTURE_NUM_COUNT},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
       MAX_GLOBAL_DESC_SET * CONFIG_STORAGE_BINDING_COUNT}};
  descriptor_pool_ = new DescriptorPool(
      shared_from_this(), VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
      pool_size, sizeof(pool_size) / sizeof(pool_size[0]), MAX_GLOBAL_DESC_SET);
}

bool VkDriver::isDeviceExtensionEnabled(const char *extension_name) {
  for (const auto &ext : enabled_device_extensions_) {
    if (strcmp(ext, extension_name) == 0)
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

  if (g_engine.getVkConfig()->getVersion() >= VK_MAKE_VERSION(1, 3, 0)) {
    vma_vulkan_func.vkGetDeviceBufferMemoryRequirements =
        vkGetDeviceBufferMemoryRequirements;
    vma_vulkan_func.vkGetDeviceImageMemoryRequirements =
        vkGetDeviceImageMemoryRequirements;
  }

  VmaAllocatorCreateInfo allocator_info{};
  allocator_info.physicalDevice = physical_device_;
  allocator_info.device = device_;
  allocator_info.instance = instance_;
  allocator_info.vulkanApiVersion = g_engine.getVkConfig()->getVersion();
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
  frames_data_.clear();
  render_targets_.clear();
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
  vkDestroyInstance(instance_, nullptr);
}
} // namespace mango
