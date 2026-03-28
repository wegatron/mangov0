# ECS 世界系统

`source/engine/functional/world/` 和 `source/engine/functional/component/` 实现了引擎的场景管理，基于 [EnTT](https://github.com/skypjack/entt) ECS 框架。

---

## 目录

1. [为什么使用 ECS](#1-为什么使用-ecs)
2. [World 类](#2-world-类)
3. [组件定义](#3-组件定义)
4. [Transform 树](#4-transform-树)
5. [场景导入流程](#5-场景导入流程)
6. [光源数据管理](#6-光源数据管理)
7. [帧 tick 流程](#7-帧-tick-流程)

---

## 1. 为什么使用 ECS

传统的面向对象游戏引擎通常使用**深度继承层级**来组织游戏对象（GameObject 继承链），这带来一些问题：

- **组合爆炸**：同时需要多种能力时，继承链越来越深
- **缓存不友好**：对象分散在堆上，遍历大量对象时 cache miss 严重
- **扩展困难**：添加新功能往往需要修改基类

ECS（Entity-Component-System）将**数据**（Component）和**行为**（System）彻底分离：

- **Entity**：只是一个 ID（无数据、无行为）
- **Component**：纯数据结构，附加在 Entity 上
- **System**：操作一类具有特定 Component 组合的 Entity

优势：
- 组合灵活：任意组合 Component 实现不同能力，无需继承
- 缓存友好：同类型 Component 连续存储（Sparse-Array 或 Archetype），遍历时 cache 命中率高
- 代码更易复用和扩展

EnTT 使用 **Sparse-Array** 实现：每种 Component 类型维护一个 sparse array + dense array，实体 ID 作为 sparse 索引，dense array 中紧密存储 Component 数据，保证遍历性能。

---

## 2. World 类

`World`（`world.h`）是引擎的场景容器，持有 EnTT `registry` 和 Transform 树根节点。

```cpp
class World final {
public:
    void tick(float seconds);           // 每帧更新

    void importScene(const std::string &url);   // 导入场景（异步）
    void saveAsWorld(const URL &url);           // 保存为引擎格式
    void saveWorld();                           // 保存到当前路径

    entt::entity createEntity(const std::string &name);
    void removeEntity(entt::entity entity);
    template<typename T> void addComponent(entt::entity, const T &comp);

    auto getCameras();       // view: string + TransformRelationship + CameraComponent
    auto getStaticMeshes();  // view: string + TransformComponent + StaticMesh + Material
    auto &getDefaultCameraComp();

    bool isLightingDirty() const;
    const ULighting& getLighting() const;
    void clearLightingDirty();

private:
    entt::registry entities_;
    std::shared_ptr<TransformRelationship> root_tr_;
    std::vector<ImportedSceneData> imported_scene_datas_[MAX_FRAMES_IN_FLIGHT];
    entt::entity default_camera_;
    ULighting lighting_;
    bool lighting_dirty_{true};
};
```

`g_engine` 通过 `getWorld()` 访问唯一的 World 实例。

---

## 3. 组件定义

引擎当前支持的组件类型（`components.h` + 各 component_xxx.h）：

| 组件 | 类型 | 说明 |
|------|------|------|
| `std::string` | 值类型 | 实体名称 |
| `TransformComponent` | `shared_ptr<TransformRelationship>` | 变换节点（位移/旋转/缩放）及父子层级 |
| `StaticMeshComponent` | `shared_ptr<StaticMesh>` | 静态网格（顶点/索引 GPU buffer） |
| `MaterialComponent` | `shared_ptr<Material>` | 材质（PBR 参数 + 贴图 + DescriptorSet） |
| `CameraComponent` | 值类型 | 透视相机：FOV、near/far、视图矩阵、ev100、Trackball 控制 |

类型别名（`components.h`）：
```cpp
using StaticMeshComponent = std::shared_ptr<StaticMesh>;
using MaterialComponent   = std::shared_ptr<Material>;
using TransformComponent  = std::shared_ptr<TransformRelationship>;
```

### CameraComponent

`CameraComponent` 实现了右手坐标系的透视投影（depth 范围 [0, 1]），并内置 Trackball 相机控制（旋转、平移、缩放）。

- 视图矩阵通过 `setLookAt()` / `setRotation()` 系列方法设置
- 投影矩阵通过 `setFovy()` / `setAspect()` / `setClipPlanes()` 设置，`dirty_proj_` 标记延迟重算
- `ev100` 曝光值传入 `ULighting` UBO（见 [render_system.md](render_system.md)）

---

## 4. Transform 树

`TransformRelationship`（`component_transform.h`）用三个指针构建场景树，以避免 vector 扩容带来的失效问题：

```cpp
struct TransformRelationship {
    std::shared_ptr<TransformRelationship> parent;
    std::shared_ptr<TransformRelationship> child;    // 第一个子节点
    std::shared_ptr<TransformRelationship> sibling;  // 下一个兄弟节点
    Eigen::Matrix4f ltransform;  // 局部变换矩阵
    Eigen::Matrix4f gtransform;  // 全局变换矩阵（世界空间）
    Eigen::AlignedBox3f aabb;    // 当前节点 mesh 的 AABB（不含子节点）
};
```

**parent / child / sibling 链表结构：**

```
root
 └─ child_A
      ├─ sibling: child_B
      │    └─ sibling: child_C
      └─ child: grandchild_A1
```

`World::updateTransform()` 每帧深度优先遍历树，将 `ltransform` 逐级累乘得到 `gtransform`（世界变换矩阵）。

---

## 5. 场景导入流程

场景导入是**异步**的，通过事件系统触发，结果按帧队列化后在 `World::tick()` 中消费：

```
MenuUI → 用户选择文件
    │
    ▼
EventSystem::asyncDispatch(ImportSceneEvent)
    │
    ▼（事件线程消费）
AssimpImporter::import(url)
    ├─ 提取 Mesh → AssetMesh → inflate() → StaticMesh (GPU buffer)
    ├─ 提取 Material → AssetMaterial → inflate() → Material (DescriptorSet)
    ├─ 提取 Texture → AssetTexture → DataUploader → VkImage
    ├─ 提取 Light → ULighting
    └─ 提取 Node 层级 → TransformRelationship 树
    │
    ▼
World::enqueue(root_tr, mesh_entity_datas, light_entity_datas, lighting)
    （写入 imported_scene_datas_[cur_frame_index]）
    │
    ▼（下一帧 World::tick() → loadedMesh2World()）
    ├─ 消费上一帧的 imported_scene_datas_
    ├─ 创建 ECS 实体
    ├─ 附加 TransformComponent / StaticMeshComponent / MaterialComponent
    ├─ 挂载到 root_tr_ 树
    └─ 更新 lighting_（置 lighting_dirty_ = true）
```

`enqueue()` 写入的是当前帧的 slot，`loadedMesh2World()` 消费**上一帧**的 slot，确保不与 GPU 渲染中的帧产生数据竞争。

---

## 6. 光源数据管理

场景导入时，Assimp 提取的光源信息被聚合为 `ULighting` 结构体（最多 8 盏方向光 + 8 盏点光源），存储在 `World::lighting_` 中。

`RenderSystem` 每帧通过 `world->isLightingDirty()` 检查，若为 true 则重新上传 Lighting UBO 到 GPU（`set=0 binding=0`），然后调用 `clearLightingDirty()`。

光度学参数详见 [render_system.md](render_system.md)。

---

## 7. 帧 tick 流程

`World::tick(dt)` 每帧由 `EngineContext::logicTick()` 调用：

```
World::tick(dt)
    ├─ loadedMesh2World()
    │    └─ 消费上一帧 ImportedSceneData 队列
    │         ├─ 创建/更新 ECS 实体和组件
    │         └─ 更新 lighting_ / lighting_dirty_
    ├─ updateTransform()
    │    └─ 深度优先遍历 TransformRelationship 树
    │         └─ gtransform = parent.gtransform × ltransform
    └─ updateCamera()
         └─ 若 focus_camera2world_ 则自动调整默认相机位置以包含整个场景
```

---

## 参考

- [ECS back and forth](https://skypjack.github.io/2019-03-07-ecs-baf-part-2/)
- [How are components/entity stored in memory? (flecs issue #3)](https://github.com/SanderMertens/flecs/issues/3)
- [EnTT documentation](https://github.com/skypjack/entt/wiki)
