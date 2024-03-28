## 参考代码/系统
vulkan-samples:
1. 以scene_graph为主, 其中的node可以添加component. 主要的component是Mesh(包含材质==renderable)
2. scene_graph只存储数据, 由各个pass(system), 进行相关事务的处理.
3. 节点之间有明确的父子关系, 通过向上查找更新节点的world transform.

filament:
1. filament中通过几个不同的Manager与entt::registry类似, 将entity分为几个大类: Renderable, Light, Camera 
2. 由FScene::prepare负责将scene中的entity转化为可渲染的集合数据mRenderableData
    主要处理light, renderable
3. filament中View来定义了一个场景的渲染配置. 比如: 相机、阴影、后处理等.
4. 在节点更新时同时更新其所有子节点. 通过child、next指针实现父子节点树(每个节点大小固定).

## Design
参考filament的思想, 使用entt实现ECS. 分为如下几类entity:
- Camera
- Light
- Renderable