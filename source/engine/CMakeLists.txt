
file(GLOB_RECURSE utils CONFIGURE_DEPENDS utils/*.h utils/*.hpp utils/*.cpp)

file(GLOB_RECURSE platform CONFIGURE_DEPENDS platform/*.h platform/*.hpp platform/*.cpp)
file(GLOB_RECURSE resource CONFIGURE_DEPENDS resource/stb_image.cpp)
file(GLOB_RECURSE asset CONFIGURE_DEPENDS asset/*.h asset/*.hpp asset/*.cpp)

file(GLOB_RECURSE global CONFIGURE_DEPENDS functional/global/*.h functional/global/*.hpp functional/global/*.cpp)
file(GLOB_RECURSE render CONFIGURE_DEPENDS functional/render/*.h functional/render/*.hpp functional/render/*.cpp)
file(GLOB_RECURSE component CONFIGURE_DEPENDS functional/component/*.h functional/component/*.hpp functional/component/*.cpp)

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
${utils}
${platform}
${global}
${asset}
${render}
${resource}
)

add_library(engine STATIC
${utils}
${platform}
${global}
${asset}
${render}
${resource}
${IMGUI_SRC}
)

target_include_directories(engine PRIVATE
    $<TARGET_PROPERTY:volk,INTERFACE_INCLUDE_DIRECTORIES>
)

# linked static libraries should not be linked again in outer target, to avoid duplicate linking
target_link_libraries(engine PRIVATE
${GLFW_LIBRARY}
glslang::glslang
glslang::glslang-default-resource-limits
glslang::SPIRV
GPUOpen::VulkanMemoryAllocator
spirv-cross-reflect
spirv-cross-core
spirv-cross-glsl
spdlog::spdlog
)