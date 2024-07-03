# Overview

## Design
![archi](imgs/mango_archi.svg)

* platform
    * file system
        provides ability to read, write, create, remove, copy file
    * window
        provides ability to create, scale window, and converts window event into a unified format.
* utils
    * vk
        vulkan wrapper, provides class encapsulation of vulkan resources, status, and commands. It also provides some auxiliary functions (such as device feature/extension config, shader compilation and parsing, resource caching, data upload and download, and thread-local command buffer management).
    * log
        log system based on spdlog.
    * event
        event system based on eventpp.
    * base
        macros and some helper functions.

实现细节:
* [Rendering System]()

## setup
run `prepare.sh` to download and build thirdparty libs.

## Reference
```mermaid
mindmap
  root((Reference))
    Using Vulkan
      vulkan-in-30-minutes https://renderdoc.org/vulkan-in-30-minutes.html    
    Vulkan Practices guide
      Arm GPU Best Practices https://developer.arm.com/documentation/101897/0302
    Architecture
        引擎架构分层 <br> https://blog.csdn.net/qq_48185715/article/details/123939639
        ECS <br> why_ecs.md
```