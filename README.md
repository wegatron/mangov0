# VK Engine
A Minimal Vulkan Render Engine for learning purpose.

[Design doc](doc/doc.md)

setup: 
* 3rd libraries, run prepare.sh
* for test download https://github.com/KhronosGroup/glTF-Sample-Models/tree/main/2.0/MetalRoughSpheres
* download [buster-drone gltf scene](https://sketchfab.com/3d-models/buster-drone-294e79652f494130ad2ab00a13fdbafd), put it to data
* build and run

## 
    https://github.com/taskflow/taskflow.git

## TODO List
- [ ] Asset Manager
- [x] basic BRDF
- [ ] area light
    reference: 
    【论文复现】Real-Time Polygonal-Light Shading with Linearly Transformed Cosines    
    https://learnopengl.com/Guest-Articles/2022/Area-Lights
    https://zhuanlan.zhihu.com/p/350694154
    textured area light: https://www.shadertoy.com/view/wlGSDK
- [ ] image based lighting
- [ ] [normal maping](https://learnopengl.com/Advanced-Lighting/Normal-Mapping)
- [ ] FXAA refer to http://blog.simonrodriguez.fr/articles/2016/07/implementing_fxaa.html
- [ ] animation
- [ ] [parallax occlusion maping](https://learnopengl.com/Advanced-Lighting/Parallax-Mapping)
- [ ] principle BSDF
- [ ] subsurface scattering
- [ ] other algorithms: https://replicability.graphics/