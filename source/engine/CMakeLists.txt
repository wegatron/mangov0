
file(GLOB_RECURSE utils_headers CONFIGURE_DEPENDS utils/*.h utils/*.hpp)
file(GLOB_RECURSE utils_srcs CONFIGURE_DEPENDS utils/*.cpp)

file(GLOB_RECURSE platform_headers CONFIGURE_DEPENDS platform/*.h platform/*.hpp)
file(GLOB_RECURSE platform_srcs CONFIGURE_DEPENDS platform/*.cpp)

file(GLOB_RECURSE global_headers CONFIGURE_DEPENDS functional/global/*.h functional/global/*.hpp)
file(GLOB_RECURSE global_srcs CONFIGURE_DEPENDS functional/global/*.cpp)

file(GLOB_RECURSE render CONFIGURE_DEPENDS functional/render/*.h functional/render/*.hpp functional/render/*.cpp)

set(IMGUI_HEADERS
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/imconfig.h
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/imgui.h
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/imgui_internal.h
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/imstb_rectpack.h
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/imstb_textedit.h
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/imstb_truetype.h
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/backends/imgui_impl_glfw.h
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/backends/imgui_impl_vulkan.h
)
set(IMGUI_SRC
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/imgui_demo.cpp
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/backends/imgui_impl_glfw.cpp
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/backends/imgui_impl_vulkan.cpp
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES 
${utils_headers} ${utils_srcs}
${platform_headers} ${platform_srcs}
${global_headers} ${global_srcs}
${render}
)

add_library(engine STATIC
    ${utils_headers} ${utils_srcs}
    ${platform_headers} ${platform_srcs}
    ${global_headers} ${global_srcs}
    ${render}
    ${IMGUI_HEADERS} ${IMGUI_SRC}
)
target_link_libraries(engine PRIVATE
volk
${GLFW_LIBRARY}
${THREAD}
glslang::glslang
glslang::glslang-default-resource-limits
glslang::SPIRV
GPUOpen::VulkanMemoryAllocator
spirv-cross-reflect
spirv-cross-core
spirv-cross-glsl
spdlog::spdlog
)