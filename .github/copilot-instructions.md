# Mango Copilot Instructions

## Build, test, and run commands

- Bootstrap third-party dependencies from the repository root with `python prepare.py`. This clones/builds libraries into `thirdparty\install` and is required before the first CMake configure on a fresh checkout.
- Configure with `cmake -B build`.
- Build the default app with `cmake --build build --config Release --target mango_editor`. `cmake --build build --config Release` also works for the full default build.
- VS Code launch settings run the executable with `run\` as the working directory. Keep that when debugging locally; `FileSystem::init()` throws immediately if it cannot find `asset\` or `..\asset` relative to the process working directory.
- Runtime shader source is loaded from `run\shaders\...` (`MainPass` hardcodes `shaders/static_mesh.vert` and `shaders/forward_lighting.frag`). The runtime SPIR-V cache is written under `run\asset\engine\shader\`.
- There is no enabled test suite in the root build today. `source\test\CMakeLists.txt` defines a standalone `zsw_test` executable, but `add_subdirectory(source/test)` is commented out in the root `CMakeLists.txt`, and there are no `enable_testing()` / `add_test()` calls. That means there is currently no supported full-suite or single-test command without first wiring that target into the root build.
- No repository lint target or formatter target is configured in CMake or repo-level docs.

## High-level architecture

- The repository builds three layers: `engine` (core static library), `editor` (ImGui editor static library), and `mango_editor` (the executable entry point in `source\samples\editor\main.cpp`).
- `engine` is split into four practical subsystems:
  - `platform`: filesystem and windowing (`FileSystem`, `GlfwWindow`)
  - `utils`: Vulkan wrappers plus logging, events, timers, and shared helpers
  - `asset`: engine-native asset loading/serialization plus Assimp-based scene import
  - `functional`: world/ECS logic, render orchestration, and global engine state
- `mango::g_engine` (`EngineContext`) is the central service locator and lifecycle owner. `Editor::run()` drives the frame in this order: `processEvents()` -> `g_engine.newTick()` -> `gcTick()` -> `logicTick()` -> `renderTick()` -> `threadSync()`.
- The engine uses a two-thread frame model. The main thread handles windowing and graphics submission. A background event/transfer thread runs `event_system_->tick()`, uses its own thread-local command buffer manager, and can submit transfer work to the transfer queue. `RenderSystem` then waits on semaphores produced by that thread before graphics submission.
- `World` is an EnTT-backed ECS container plus a transform tree (`TransformRelationship`). Scene import is asynchronous in practice: `ImportSceneEvent` triggers `AssimpImporter`, imported data is queued per frame in `World::enqueue()`, and `World::loadedMesh2World()` consumes the previous frame's queue and materializes entities/components into the ECS.
- `RenderSystem` gathers `RenderData` from the ECS every frame, updates the global lighting UBO only when `World` marks lighting dirty, renders the 3D scene through `MainPass`, then overlays the ImGui editor through `UIPass`.
- Descriptor usage is fixed by convention: set `0` is the global lighting UBO managed by `ResourceBindingMgr`, set `1` is the material descriptor set created by `Material::inflate()`, and per-object transforms are sent via the `TransformPCO` push constant.

## Key conventions

- Prefer accessing engine services through `g_engine` getters instead of threading new subsystem references through unrelated classes. Most of the codebase assumes that pattern for the driver, resource cache, event system, filesystem, world, and render system.
- Do not treat asset deserialization as the end of the asset pipeline. GPU-backed assets become usable only after their `inflate()` step allocates buffers/images/descriptors. `Material::inflate()` is the canonical example: it allocates a standard material descriptor set and writes the UBO + texture bindings through `ResourceBindingMgr`.
- Preserve the event-driven flow when adding editor or engine interactions. Window/input and editor actions are commonly converted into typed events and consumed through `EventSystem` listeners rather than direct cross-module calls.
- Be careful with frame ownership and synchronization. World imports are staged per frame, and transfer-thread submissions are synchronized into graphics work through `RenderSystem` semaphore bookkeeping. Changes in these areas need to respect the existing per-frame handoff instead of introducing immediate cross-thread mutations.
- Runtime paths are working-directory-sensitive. `run\asset\...` is not optional, and several systems use filesystem-relative paths rather than repo-root-relative paths.
- Shader path handling is not fully centralized yet: `MainPass` currently hardcodes `shaders\...` paths while `FileSystem::getShaderDir()` returns `shader` singular. If you touch shader loading, inspect both call sites before normalizing paths.
