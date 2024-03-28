#include <engine/utils/base/macro.h>
#include <iostream>

#include <cassert>
#include <volk.h>
#include <vulkan/vulkan.h>

#include <engine/functional/global/engine_context.h>
#include <engine/platform/glfw_window.h>
#include <engine/utils/vk/vk_config.h>

int main(int argc, char const *argv[]) {
  auto vk_config = std::make_shared<mango::Vk13Config>();
  vk_config->setDeviceType(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
  vk_config->setFeatureEnabled(
      mango::VkConfig::FeatureExtension::GLFW_EXTENSION,
      mango::VkConfig::EnableState::REQUIRED);
  vk_config->setFeatureEnabled(mango::VkConfig::FeatureExtension::KHR_SWAPCHAIN,
                               mango::VkConfig::EnableState::REQUIRED);
  vk_config->setFeatureEnabled(
      mango::VkConfig::FeatureExtension::
          INSTANCE_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2,
      mango::VkConfig::EnableState::REQUIRED);
#if !NDEBUG
  vk_config->setFeatureEnabled(
      mango::VkConfig::FeatureExtension::KHR_VALIDATION_LAYER,
      mango::VkConfig::EnableState::REQUIRED);
#endif

  auto window = std::make_shared<mango::GlfwWindow>("viewer", 800, 600);

  mango::g_engine.init(vk_config, window);
  while (!window->shouldClose()) {
    window->processEvents();
  }
  return 0;
}