#include <IconsFontAwesome5.h>
#include <engine/functional/global/engine_context.h>
#include <engine/functional/render/pass/ui_pass.h>
#include <engine/platform/file_system.h>
#include <engine/platform/glfw_window.h>
#include <engine/utils/base/macro.h>
#include <engine/utils/base/timer.h>
#include <engine/utils/event/event_system.h>
#include <engine/utils/vk/commands.h>
#include <engine/utils/vk/descriptor_set.h>
#include <engine/utils/vk/pipeline.h>
#include <engine/utils/vk/render_pass.h>
#include <engine/utils/vk/resource_cache.h>
#include <engine/utils/vk/vk_driver.h>
#include <engine/utils/vk/swapchain.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_vulkan.h>
#include <imgui/imgui.h>
#include <iostream>

namespace mango {

void check_vk_result(VkResult err) { VK_ASSERT(err, "Imgui init error"); }

void UIPass::init() {
  createRenderPassAndFramebuffer();
  initImgui();
}

UIPass::~UIPass() {
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void UIPass::initImgui() {
  StopWatch stop_watch;
  stop_watch.start();
  // setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  // io.IniFilename = nullptr;

  // setup Dear ImGui style
  ImGui::StyleColorsDark();
  // ImGui::StyleColorsLight();
  ImGui::GetStyle().WindowMenuButtonPosition = ImGuiDir_None;

  ImGui_ImplGlfw_InitForVulkan(
      reinterpret_cast<GLFWwindow *>(g_engine.getWindow()->getHandle()), true);

  auto driver = g_engine.getDriver();
  auto queue = driver->getGraphicsQueue();
  ImGui_ImplVulkan_InitInfo init_info = {};
  init_info.Instance = driver->getInstance();
  init_info.PhysicalDevice = driver->getPhysicalDevice();
  init_info.Device = driver->getDevice();

  init_info.QueueFamily = queue->getFamilyIndex();
  init_info.Queue = queue->getHandle();
  init_info.PipelineCache = g_engine.getResourceCache()->getPipelineCache();
  init_info.DescriptorPool = driver->getDescriptorPool()->getHandle();
  init_info.Allocator = nullptr;
  init_info.MinImageCount = MAX_FRAMES_IN_FLIGHT;
  init_info.ImageCount = MAX_FRAMES_IN_FLIGHT;
  init_info.CheckVkResultFn = check_vk_result;
  init_info.RenderPass = render_pass_->getHandle();

  // 若要与volk一起使用的话, 参考: https://zhuanlan.zhihu.com/p/634912614
  auto instance = driver->getInstance();
  ImGui_ImplVulkan_LoadFunctions(
      [](const char *function_name, void *vulkan_instance) {
        return vkGetInstanceProcAddr(
            *(reinterpret_cast<VkInstance *>(vulkan_instance)), function_name);
      },
      &instance);
  bool is_success = ImGui_ImplVulkan_Init(&init_info); // render_pass_
  MANGO_ASSERT(is_success, "failed to init imgui");

  // 加载各种font
  // add consola font
  const float k_base_font_size = 14.0f;
  const float k_icon_font_size = k_base_font_size * 0.8f;
  const auto &fs = g_engine.getFileSystem();
  io.Fonts->AddFontFromFileTTF(
      fs->absolute("asset/engine/font/consola.ttf").c_str(), k_base_font_size);

  // add icon font
  ImFontConfig icons_config;
  icons_config.MergeMode = true;
  icons_config.PixelSnapH = true;
  icons_config.GlyphMinAdvanceX = k_icon_font_size;
  static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_16_FA, 0};
  io.Fonts->AddFontFromFileTTF(
      fs->absolute("asset/engine/font/fa-solid-900.ttf").c_str(),
      k_icon_font_size, &icons_config, icons_ranges);

  // add small consola font
  const float k_small_font_size = 12.0f;
  io.Fonts->AddFontFromFileTTF(
      fs->absolute("asset/engine/font/consola.ttf").c_str(), k_small_font_size);

  // add big icon font
  const float k_big_icon_font_size = 18.0f;
  icons_config.MergeMode = false;
  icons_config.GlyphMinAdvanceX = k_big_icon_font_size;
  io.Fonts->AddFontFromFileTTF(
      fs->absolute("asset/engine/font/fa-solid-900.ttf").c_str(),
      k_big_icon_font_size, &icons_config, icons_ranges);
  LOGI("ui pass init time: {}ms", stop_watch.stopMs());
}

void UIPass::createRenderPassAndFramebuffer() {
  const auto &driver = g_engine.getDriver();
  const auto swapchain = driver->getSwapchain();
  auto color_format = swapchain->getImageFormat();
  std::vector<Attachment> attachments{Attachment{
      color_format, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_UNDEFINED}};
  std::vector<LoadStoreInfo> load_store_infos{
      {VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE}};
  std::vector<SubpassInfo> subpass_infos{{
      {},  // no input attachment
      {0}, // color attachment index
      {},  // no msaa
           // 1    // depth stencil attachment index
  }};
  render_pass_ = std::make_shared<RenderPass>(driver, attachments,
                                              load_store_infos, subpass_infos);
  driver->getSwapchain()->createFrameBuffer(render_pass_);
}

void UIPass::prepare() {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  // set docking over viewport
  ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
  ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), dockspace_flags);

  // construct imgui widgets
  g_engine.getEventSystem()->syncDispatch(
      std::make_shared<RenderConstructUIEvent>());

  ImGui::Render(); // prepare render data and command
}

void UIPass::render(const std::shared_ptr<CommandBuffer> &cmd_buffer) {
  auto driver = g_engine.getDriver();
  auto cur_img_index = driver->getCurImageIndex();
  auto frame_buffer = driver->getSwapchain()->getFrameBuffer(cur_img_index);
  cmd_buffer->beginRenderPass(render_pass_, frame_buffer);

  ImDrawData *draw_data = ImGui::GetDrawData();
  ImGui_ImplVulkan_RenderDrawData(draw_data, cmd_buffer->getHandle());

  cmd_buffer->endRenderPass();

  // image barrier
  ImageMemoryBarrier barrier{
      .old_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .new_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
      .src_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
      .dst_access_mask = 0,
      .src_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      .dst_stage_mask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
      .src_queue_family_index = VK_QUEUE_FAMILY_IGNORED,
      .dst_queue_family_index = VK_QUEUE_FAMILY_IGNORED};
  cmd_buffer->imageMemoryBarrier(
      barrier,
      frame_buffer->getRenderTarget()->getImageViews()[0]);
}

void UIPass::onCreateSwapchainObject(const uint32_t width,
                                     const uint32_t height) {
  g_engine.getDriver()->getSwapchain()->createFrameBuffer(render_pass_);
}

} // namespace mango