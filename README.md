# Mango

A minimal Vulkan-based rendering engine for learning purposes. Implements a forward-rendering pipeline with PBR materials, ECS-based scene management, and an ImGui editor UI.

## Documentation

- [Architecture](docs/architecture.md) — Module overview, layer breakdown, main loop, render pipeline, build guide
- [Render System](docs/render_system.md) — Lighting model, camera exposure, coordinate system, shader bindings
- [Vulkan](docs/vulkan.md) — Vulkan wrapper internals: initialization, pipeline, synchronization
- [ECS World](docs/ecs_world.md) — ECS scene system: components, transform tree, async scene import
- [Build Troubleshooting](docs/build_troubleshooting.md) — Known compile errors and fixes

## Features

- **Vulkan rendering** via a thin C++ wrapper (VkDriver, ResourceCache, custom command buffer management)
- **Forward lighting pass** with PBR material support (albedo, normal, emissive, metallic/roughness/occlusion maps)
- **ECS scene management** via [EnTT](https://github.com/skypjack/entt)
- **Trackball camera** with physically-based exposure (EV100)
- **Asset pipeline**: import glTF / FBX / OBJ via Assimp → engine-native format (cereal serialization)
- **Dual-thread architecture**: render thread + event/transfer thread
- **ImGui editor**: viewport, scene hierarchy, asset browser, property inspector, log panel
- **Runtime GLSL→SPIRV compilation** via glslang + SPIRV reflection via spirv-cross

## Setup

### Prerequisites

- CMake ≥ 3.20
- C++20 compiler (MSVC 2022, Clang 15+, or GCC 11+)
- Vulkan SDK

### Build

```bash
# 1. Download and build third-party libraries
python prepare.py

# 2. Configure and build
cmake -B build
cmake --build build --config Release
```

### Run

The editor's working directory is `run/` (configured in `.vscode/launch.json`). The engine looks for an `asset/` directory relative to the working directory at startup — if it is not found the engine throws immediately.

**Required directory structure:**

```
run/
├── shaders/                        # copied from project shaders/
│   ├── static_mesh.vert
│   ├── forward_lighting.frag
│   └── include/
│       ├── constants.h
│       ├── material.h
│       ├── pbr.h
│       └── shader_structs.h
└── asset/
    └── engine/
        ├── font/
        │   ├── consola.ttf         # from C:\Windows\Fonts\
        │   └── fa-solid-900.ttf    # from Font Awesome 5 Free webfonts
        ├── texture/ui/             # 11 icon PNGs (see asset pack below)
        │   ├── invalid.png
        │   ├── texture_2d.png
        │   ├── texture_cube.png
        │   ├── material.png
        │   ├── skeleton.png
        │   ├── static_mesh.png
        │   ├── skeletal_mesh.png
        │   ├── animation.png
        │   ├── world.png
        │   ├── empty_folder.png
        │   └── non_empty_folder.png
        ├── world/                  # world template files (see asset pack below)
        │   ├── empty.world
        │   ├── empty.png
        │   ├── basic.world
        │   └── basic.png
        └── shader/                 # auto-created at runtime (SPIR-V cache)
```

`log/` and `cache/` directories are created automatically on first run.

**Shader files** are compiled at runtime from GLSL source by glslang. Copy them from the project root:

```bash
xcopy /E /I shaders run\shaders
```

**Font files:**
- `consola.ttf` — copy from `C:\Windows\Fonts\consola.ttf`
- `fa-solid-900.ttf` — download from [Font Awesome 5 Free](https://github.com/FortAwesome/Font-Awesome/raw/5.15.4/webfonts/fa-solid-900.ttf)

**Engine asset pack** (UI icons, world templates): download from [Google Drive](https://drive.google.com/file/d/1GvQA9yCLWtsdc3wsg7r0WjV07JqKn63A/view?usp=sharing) and extract into `run/`.

**3D scene models** (optional, for import via File → Import Scene):
- [MetalRoughSpheres](https://github.com/KhronosGroup/glTF-Sample-Models/tree/main/2.0/MetalRoughSpheres) — glTF PBR material test scene
- [Buster Drone](https://sketchfab.com/3d-models/buster-drone-294e79652f494130ad2ab00a13fdbafd) — complex glTF scene

Place downloaded models anywhere accessible and open them via **File → Import Scene** in the editor.

## TODO

### Rendering
- [x] Static mesh rendering (vertex transform, basic pass)
- [x] Trackball camera
- [x] PBR material data pipeline
- [ ] BRDF — point light
- [ ] BRDF — directional light
- [ ] Normal mapping ([LearnOpenGL](https://learnopengl.com/Advanced-Lighting/Normal-Mapping))
- [ ] Parallax occlusion mapping ([LearnOpenGL](https://learnopengl.com/Advanced-Lighting/Parallax-Mapping))
- [ ] FXAA ([reference](http://blog.simonrodriguez.fr/articles/2016/07/implementing_fxaa.html))

### Global Illumination & Lighting
- [ ] IBL (Image-Based Lighting)
- [ ] Shadow mapping
- [ ] Area lights (LTC — [paper](https://learnopengl.com/Guest-Articles/2022/Area-Lights), [reference impl](https://www.shadertoy.com/view/wlGSDK))
- [ ] Subsurface scattering

### Animation
- [ ] Skeletal animation

### Reference
- [replicability.graphics](https://replicability.graphics/) — rendering algorithm reference implementations