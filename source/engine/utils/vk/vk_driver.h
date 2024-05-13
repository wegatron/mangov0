#pragma once

#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <vk_mem_alloc.h>
#include <volk.h>
#include <thread>

#include <engine/utils/vk/vk_config.h>
#include <engine/utils/vk/vk_constants.h>

namespace mango {

class CommandQueue;
class Swapchain;
class RenderTarget;
class DescriptorPool;
class StagePool;
class CommandPool;
class Semaphore;
class Fence;

struct RequestedDeviceExtension {
  const char *name;
  bool required;
};

struct FrameDataGlobal {  
  std::shared_ptr<Semaphore> image_available_semaphore[MAX_FRAMES_IN_FLIGHT];
  std::shared_ptr<Semaphore> render_result_available_semaphore[MAX_FRAMES_IN_FLIGHT];
  void destroy() {
    for (auto i = 0; i<MAX_FRAMES_IN_FLIGHT; ++i) {
       image_available_semaphore[i].reset();
       render_result_available_semaphore[i].reset();
    }
  }
};

struct FrameDataThreadLocal {
  std::thread::id tid;
  std::shared_ptr<Fence> command_buffer_available_fence[MAX_FRAMES_IN_FLIGHT];
  std::shared_ptr<CommandPool> command_pool[MAX_FRAMES_IN_FLIGHT];
  void destroy()
  {
    for (auto i = 0; i<MAX_FRAMES_IN_FLIGHT; ++i) {
      command_buffer_available_fence[i].reset();
      command_pool[i].reset();
    }
  }
};

class VkDriver final : public std::enable_shared_from_this<VkDriver> {
public:
  VkDriver() = default;
  ~VkDriver() = default;

  VkDriver(const VkDriver &) = delete;
  VkDriver &operator=(const VkDriver &) = delete;

  /**
   * @brief init vulkan instance -> create window surface -> select physical
   * device -> create logical device and command queue; create vma allocator;
   * create swapchain and render targets.
   */
  void init();

  void initCommandPoolForThread(const uint32_t queue_family_index);

  /**
   * @brief destroy all resources created by this driver, resource may have
   * driver's shared_ptr
   */
  void destroy();

  VkInstance getInstance() const noexcept { return instance_; }

  VkPhysicalDevice getPhysicalDevice() const { return physical_device_; }

  VkDevice getDevice() const { return device_; }

  VmaAllocator getAllocator() const { return allocator_; }

  DescriptorPool *getDescriptorPool() const { return descriptor_pool_; }

  CommandQueue *getGraphicsQueue() const { return graphics_cmd_queue_; }

  CommandQueue *getTransferQueue() const { return transfer_cmd_queue_; }

  /**
   * @brief request command buffer from main render thread
   * command buffer is allocated from command pool of current frame, when new frame is started pool will be reset
   *    
   * @param level command buffer level
   * @return std::shared_ptr<class CommandBuffer> 
   */
  std::shared_ptr<class CommandBuffer> requestCommandBuffer(
      VkCommandBufferLevel level =
          VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY);

  VkResult waitIdle() const { return vkDeviceWaitIdle(device_); }

  VkFormat getSwapchainImageFormat() const;

  uint32_t getCurImageIndex() const { return cur_frame_index_; }

  const std::shared_ptr<RenderTarget> * getRenderTargets() const {
    return render_targets_;
  }

  std::shared_ptr<Semaphore> getImageAvailableSemaphore() const {
    return frames_data_g_.image_available_semaphore[cur_frame_index_];
  }

  std::shared_ptr<Semaphore> getRenderResultAvailableSemaphore() const {
    return frames_data_g_.render_result_available_semaphore[cur_frame_index_];  
  }

  std::shared_ptr<Fence> getCommandBufferAvailableFence() const
  {
    auto frame_data_l = getFrameDataL();
    return frame_data_l->command_buffer_available_fence[cur_frame_index_];
  }

  /**
   * @brief accquire next image from swapchain,
   * update frame_index which will be used in acquire command buffer
   * update image_index/frame buffer index which will be used in ui pass
   */
  bool waitFrame();

  /**
   * @brief do present to swapchain
   */
  void presentFrame();

  void update(const std::vector<VkWriteDescriptorSet> &descriptor_writes) {
    vkUpdateDescriptorSets(device_,
                           static_cast<uint32_t>(descriptor_writes.size()),
                           descriptor_writes.data(), 0, nullptr);
  }

  StagePool *getStagePool() const { return stage_pool_; }

private:

  const FrameDataThreadLocal * getFrameDataL() const;

  void initInstance();

  std::pair<bool, uint32_t> selectPhysicalDevice(
      const std::vector<RequestedDeviceExtension> &request_extensions);

  void initDevice();

  void initAllocator();

  void checkSwapchainAbility();

  /**
   * @brief Create a Swapchain, and render targets.
   */
  void createSwapchain();

  /**
   * @brief Create data belong to each frame, including command pool, semaphore,
   * fence.
   */
  void createFramesData();

  void createDescriptorPool();

  void setupDebugMessenger();

  bool isDeviceExtensionEnabled(const char *extension_name);

  VkInstance instance_{VK_NULL_HANDLE};
  VkPhysicalDevice physical_device_{VK_NULL_HANDLE};
  VkDevice device_{VK_NULL_HANDLE};
  VkSurfaceKHR surface_{VK_NULL_HANDLE};

  std::vector<const char *> enabled_device_extensions_;

  VmaAllocator allocator_{VK_NULL_HANDLE};

  VkDebugUtilsMessengerEXT debug_messenger_;

  uint32_t cur_frame_index_{0};
  uint32_t cur_image_index_{0};
  
  FrameDataGlobal frames_data_g_;
  std::vector<FrameDataThreadLocal> frames_data_l_;

  CommandQueue *graphics_cmd_queue_{nullptr};
  CommandQueue *transfer_cmd_queue_{nullptr};
  Swapchain *swapchain_{nullptr};
  std::shared_ptr<RenderTarget> render_targets_[MAX_FRAMES_IN_FLIGHT];  
  DescriptorPool *descriptor_pool_{nullptr};
  StagePool *stage_pool_{nullptr};
};
} // namespace mango