#include <engine/functional/global/engine_context.h>
#include <engine/functional/render/pass/ui_pass.h>
#include <engine/functional/render/render_system.h>
#include <engine/utils/event/event_system.h>
#include <engine/utils/vk/commands.h>

namespace mango {

void RenderSystem::init() {
  // init render pass
  ui_pass_ = std::make_unique<UIPass>();
  ui_pass_->init();
}

void RenderSystem::onCreateSwapchainObjects(
    const std::shared_ptr<class Event> &event) {
  const RenderCreateSwapchainObjectsEvent *p_event =
      static_cast<const RenderCreateSwapchainObjectsEvent *>(event.get());
  ui_pass_->onResize(p_event->width, p_event->height);
}

void RenderSystem::tick(float delta_time) {
  // collect render datas
  auto driver = g_engine.getDriver();
  driver->waitFrame();

  auto cmd_buffer = driver->requestCommandBuffer(
      VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY);
  cmd_buffer->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
  ui_pass_->draw(cmd_buffer);
  cmd_buffer->end();
  auto cmd_queue = driver->getGraphicsQueue();
  cmd_queue->submit(cmd_buffer,
                    driver->getCurrentFrameData().render_fence->getHandle());
  driver->presentFrame();
}

// void Render::render(World *scene, Gui * gui)
// {
//   assert(scene != nullptr);
//   scene->update(cur_time_);

//   // update camera
//   auto &camera_manager = scene->camera_manager();
//   auto view_camera =
//   camera_manager.view<std::shared_ptr<TransformRelationship>,
//                                          Camera>();
//   const auto &cam_tr =
//   camera_manager.get<std::shared_ptr<TransformRelationship>>(*view_camera.begin());
//   auto &cam = camera_manager.get<Camera>(*view_camera.begin());
//   auto &global_param_set = getDefaultAppContext().global_param_set;
//   global_param_set->setCameraParam(cam.getCameraPos(), cam.ev100(),
//   cam.getViewMatrix() /* cam_tr->gtransform */, cam.getProjMatrix());

//   auto &lm = scene->light_manager();
//   auto lv = lm.view<std::shared_ptr<TransformRelationship>, Light>();
//   Lights lights;
//   lights.lights_count = std::distance(lv.begin(), lv.end());
//   assert(lights.lights_count <= MAX_LIGHTS_COUNT);
//   uint32_t light_index = 0;
//   for(auto &&[entity, tr, l] : lv.each())
//   {
//     lights.l[light_index] = l;
//     // Eigen::Vector4f tp;
//     // tp.head(3) = l.position; tp[3] = 1.0f;
//     // lights.l[light_index].position = (tr->gtransform * tp).head(3);
//   }
//   global_param_set->setLights(lights);
//   global_param_set->update();

//   auto &rm = scene->renderableManager();
//   auto view = rm.view<std::shared_ptr<TransformRelationship>,
//                       std::shared_ptr<Material>,
//                       std::shared_ptr<StaticMesh>>();
//   rpass_.gc();
//   auto width = frame_buffers_[cur_rt_index_]->getWidth();
//   auto height = frame_buffers_[cur_rt_index_]->getHeight();
//   cmd_buf_->beginRenderPass(rpass_.getRenderPass(),
//   frame_buffers_[cur_rt_index_]); view.each(
//       [this, width, height](const std::shared_ptr<TransformRelationship> &tr,
//                             const std::shared_ptr<Material> &mat,
//                             const std::shared_ptr<StaticMesh> &mesh) {
//         // update materials
//         mat->updateParams();

//         // // debug
//         // static float total_time = 0.0f;
//         // total_time += cur_time_;
//         // tr->gtransform.block<3, 3>(0, 0) =
//         //     Eigen::AngleAxisf(total_time, Eigen::Vector3f::UnitX()) *
//         Eigen::AngleAxisf(3.1415926f*0.5f, Eigen::Vector3f::UnitY())
//         //         .toRotationMatrix();
//         rpass_.draw(mat, tr->gtransform, mesh, cmd_buf_, width, height);
//       });
//   cmd_buf_->endRenderPass();

//   // image memory barrier
//   ImageMemoryBarrier barrier{
//       .old_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
//       .new_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
//       .src_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
//       .dst_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
//       .src_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
//       .dst_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
//       .src_queue_family_index = VK_QUEUE_FAMILY_IGNORED,
//       .dst_queue_family_index = VK_QUEUE_FAMILY_IGNORED};
//   cmd_buf_->imageMemoryBarrier(barrier,
//                                frame_buffers_[cur_rt_index_]->getRenderTarget()->getImageViews()[0]);
//   // render gui
//   // gui->update(cmd_buf_, cur_time_, cur_frame_index_, cur_rt_index_);
// }

// void Render::endFrame() {
//   // add a barrier to transition the swapchain image from color attachment to
//   // present
//   ImageMemoryBarrier barrier{
//       .old_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
//       .new_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
//       .src_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
//       .dst_access_mask = 0,
//       .src_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
//       .dst_stage_mask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
//       .src_queue_family_index = VK_QUEUE_FAMILY_IGNORED,
//       .dst_queue_family_index = VK_QUEUE_FAMILY_IGNORED};
//   auto &frame_data = getDefaultAppContext().frames_data[cur_rt_index_];
//   cmd_buf_->imageMemoryBarrier(barrier,
//                                frame_data.render_tgt->getImageViews()[0]);
//   cmd_buf_->end();
//   auto cmd_queue = getDefaultAppContext().driver->getGraphicsQueue();
//   auto &sync = getDefaultAppContext().render_output_syncs[cur_frame_index_];
//   auto cmd_buf_handle = cmd_buf_->getHandle();
//   auto present_semaphore = sync.present_semaphore->getHandle();
//   auto render_semaphore = sync.render_semaphore->getHandle();
//   auto render_fence = sync.render_fence->getHandle();
//   VkPipelineStageFlags wait_stage{
//       VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
//   VkSubmitInfo submit_info{VK_STRUCTURE_TYPE_SUBMIT_INFO};
//   submit_info.commandBufferCount = 1;
//   submit_info.pCommandBuffers = &cmd_buf_handle;
//   submit_info.waitSemaphoreCount = 1;
//   submit_info.pWaitSemaphores = &present_semaphore;
//   submit_info.pWaitDstStageMask = &wait_stage;
//   submit_info.signalSemaphoreCount = 1;
//   submit_info.pSignalSemaphores = &render_semaphore;
//   cmd_queue->submit({submit_info}, render_fence);
// }
} // namespace mango