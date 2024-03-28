### App
| 类 |  描述 |
| --- | --- |
| Window (framework/platform/window.h) | 窗口的抽象基类. 由其负责窗口的创建、关闭、移动、缩放, imgUI的初始化接入, 接收用户的UI操作产生输入事件, 创建surface. |
| AppBase (framework/utils/app_base.h) | 应用程序的抽象基类. 实现了应用的真正逻辑, 包括初始化、每一帧的tick, 以及事件的处理. |
| AppManager (framework/utils/window_app.h) | 将Window和AppBase组装成一个完整的功能类 |
| AppContext (framework/utils/app_context.h) | 全局单例, 保存了整个系统的一些全局对象: driver, descriptor_pool(GLOBAL_SET_INDEX), stage_pool ...|

```mermaid
---
title: App
---
classDiagram
    class Window {
        virtual VkSurfaceKHR createSurface(VkInstance instance)

        virtual bool shouldClose()

        virtual void getExtent(uint32_t &width, uint32_t &height) const

        virtual void processEvents() //接收用户操作, 产生ui event

        virtual void initImgui() // 这里将ui事件与接入imgui, 这里可以全部转移到callback中(待优化)

        virtual void shutdownImgui()

        virtual void imguiNewFrame()

        virtual void setupCallback(AppBase * app) // ui event callback
    }
    
    class GlfwWindow

    class AppBase {
        AppBase(const std::string &name)
        virtual void tick(const float seconds, const uint32_t rt_index, const uint32_t frame_index)
        virtual void init(Window * window, const std::shared_ptr< VkDriver > &driver, const std::vector<std::shared_ptr<RenderTarget>> &rts)
        virtual void updateRts(const std::vector< std::shared_ptr< RenderTarget > > &rts)
        virtual void inputMouseEvent(const std::shared_ptr<MouseInputEvent> &mouse_event)
    }

    class ViewerApp

    class AppManager {
        bool init(VkFormat color_format, VkFormat ds_format)

        void setApp(std::shared_ptr<AppBase> &&app)
        
        void run()
    }
    
    class AppContext {
        std::shared_ptr< VkDriver > driver
        std::shared_ptr< DescriptorPool > descriptor_pool
        std::shared_ptr< StagePool > stage_pool
        std::shared_ptr< GPUAssetManager > gpu_asset_manager
        std::shared_ptr< ResourceCache > resource_cache
        std::vector< FrameData> frames_data
        std::unique_ptr< GlobalParamSet > global_param_set
        std::vector< RenderOutputSync > render_output_syncs        
    }

    Window <|-- GlfwWindow
    AppBase <|-- ViewerApp
    AppManager --o AppBase : 1
    AppManager --o Window : 1
    
```