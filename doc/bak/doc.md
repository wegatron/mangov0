# vk_engine

doc use mermaid. 
[comparation between graphviz and mermaid](https://www.devtoolsdaily.com/diagrams/graphviz_vs_mermaidjs/)
[class diagram](https://mermaid.js.org/syntax/classDiagram.html)

vulkan quick start: [vulkan-in-30-minutes](https://renderdoc.org/vulkan-in-30-minutes.html)

## framework Modules
| 模块 |  描述 |
| --- | --- |
| framework/platform | 与系统/平台相关的类: 窗口、UI事件的抽象以及实现等|
| framework/functional | 场景相关的数据类和操作类, 包括场景组件(相机、材质、mesh), 场景的加载、渲染. |
| framework/utils | 一些通用的功能函数、类, 例如: compiler marcos、memory allocator. vk-vulkan的上层封装. 给上层提供更简单的接口, 让上层以对象的方式管理vulkan资源, 并提供了一些功能类(例如Resource cache、stagepool), 以提升系统性能. |
| framework/resources |

数据绑定规约:

| descriptor set |  用途 |
| --- | --- |
| GLOBAL_SET_INDEX = 0 | 用来存一些全局的属性/参数(对场景中的所有物体有效的参数), 例如: 相机的参数. |
| MATERIAL_SET_INDEX = 1 | 用来存可能对一个或多个物体的属性/参数, 例如: 材质. |
| OBJECT_SET_INDEX = 2 | 用来存只对一个物体有效的参数. 例如: 物体的Rt. |


### [Vulkan](vulkan.md)

### [Scene](scene.md)

### [App](app.md)

### Performace

使用`alignas`来要求数据对齐, `string_view`来替代`std::string`.
[taskflow](https://github.com/taskflow/taskflow)

## Reference
https://zeux.io/2020/02/27/writing-an-efficient-vulkan-renderer/