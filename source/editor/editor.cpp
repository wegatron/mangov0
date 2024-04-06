#include <editor/asset/asset_ui.h>
#include <editor/editor.h>
// #include "editor/global/editor_context.h"
// #include "editor/log/log_ui.h"
// #include <editor/menu/menu_ui.h>
// #include "editor/property/property_ui.h"
// #include "editor/simulation/simulation_ui.h"
// #include "editor/tool/tool_ui.h"
// #include "editor/world/world_ui.h"
#include <engine/functional/global/engine_context.h>
#include <engine/platform/window.h>
#include <engine/utils/base/macro.h>
#include <engine/utils/event/event_system.h>
#include <engine/utils/vk/commands.h>

namespace mango {
void Editor::init() {
  // init engine
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
  g_engine.init(vk_config, "editor");

  // create editor ui
  // std::shared_ptr<EditorUI> menu_ui = std::make_shared<MenuUI>();
  // std::shared_ptr<EditorUI> tool_ui = std::make_shared<ToolUI>();
  // std::shared_ptr<EditorUI> world_ui = std::make_shared<WorldUI>();
  // std::shared_ptr<EditorUI> property_ui = std::make_shared<PropertyUI>();
  std::shared_ptr<EditorUI> asset_ui = std::make_shared<AssetUI>();
  // std::shared_ptr<EditorUI> log_ui = std::make_shared<LogUI>();
  // m_simulation_ui = std::make_shared<SimulationUI>();
  // m_editor_uis = {menu_ui,  tool_ui,         world_ui, property_ui,
  //                 asset_ui, m_simulation_ui, log_ui};
  editor_uis_ = {asset_ui};
  // init all editor uis
  for (auto &editor_ui : editor_uis_) {
    editor_ui->init();
  }

  // set construct ui function to UIPass through RenderSystem
  g_engine.getEventSystem()->addListener(
      EEventType::RenderConstructUI,
      [this](const EventPointer &event) { constructUI(); });
}

void Editor::destroy() {
  // wait all gpu operations done
  auto driver = g_engine.getDriver();
  driver->getGraphicsQueue()->waitIdle();
  editor_uis_.clear();

  // destroy engine
  g_engine.destroy();
}

void Editor::run() {
  auto window = g_engine.getWindow();
  while (!window->shouldClose()) {
    window->processEvents();
    float delta_time = g_engine.calcDeltaTime();
    g_engine.logicTick(delta_time);
    g_engine.renderTick(delta_time);
  }
}

void Editor::constructUI() {
  //   if (g_editor.isSimulationPanelFullscreen()) {
  //     m_simulation_ui->construct();
  //   } else {
  //     for (auto &editor_ui : m_editor_uis) {
  //       editor_ui->construct();
  //     }
  //   }
  for (auto &editor_ui : editor_uis_) {
    editor_ui->construct();
  }
}

} // namespace mango