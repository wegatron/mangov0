cmake_minimum_required(VERSION 3.20)
project(mango VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")

# 设置编译输出的路径
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(WIN32)
    set(GLFW_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/install/lib/glfw3.lib)
    add_definitions(-DNOMINMAX)
else()
    set(GLFW_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/install/lib/libglfw3.a)
endif()

list(APPEND CMAKE_PREFIX_PATH 
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/install/lib/cmake/glslang
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/install/lib/cmake/VulkanMemoryAllocator
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/install/share/spirv_cross_reflect/cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/install/share/spirv_cross_core/cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/install/share/spirv_cross_glsl/cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/install/lib/cmake/assimp-5.3
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/install/lib/cmake/eventpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/install lib/cmake/spdlog
)

find_package(glslang REQUIRED)

find_package(VulkanMemoryAllocator CONFIG REQUIRED)

find_package(spirv_cross_reflect REQUIRED)

find_package(spirv_cross_core REQUIRED)

find_package(spirv_cross_glsl REQUIRED)

find_package(assimp REQUIRED)

find_package(eventpp REQUIRED)

find_package(spdlog REQUIRED)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/source
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/install/include
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glm
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/Vulkan-Hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/eigen
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/stbi
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty # for imgui    
)

if (NOT APPLE AND ${CMAKE_CXX_COMPILER} MATCHES "(C|c?)lang")
    add_link_options(-stdlib=libc++)
    add_compile_options(-stdlib=libc++)
endif()

add_definitions(-DVK_NO_PROTOTYPES -DGLFW_INCLUDE_VULKAN)
if (WIN32)
   set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
elseif()
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_XLIB_KHR)
endif()
add_subdirectory(thirdparty/volk)

if (NOT WIN32)
set(THREAD "-lpthread")
endif()

# include(macros.cmake)

# set(GLSL_C)
# data_to_c_simple(shaders/basic.frag GLSL_C)

add_subdirectory(source/engine)
add_subdirectory(source/editor)
#add_subdirectory(source/samples/editor)