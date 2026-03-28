
file(GLOB_RECURSE utils CONFIGURE_DEPENDS utils/*.h utils/*.hpp utils/*.cpp)

file(GLOB_RECURSE platform CONFIGURE_DEPENDS platform/*.h platform/*.hpp platform/*.cpp)
file(GLOB_RECURSE resource CONFIGURE_DEPENDS resource/stb_image.cpp)
file(GLOB_RECURSE asset CONFIGURE_DEPENDS asset/*.h asset/*.hpp asset/*.cpp)

file(GLOB_RECURSE functional CONFIGURE_DEPENDS functional/*)

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
    ${CMAKE_SOURCE_DIR}/thirdparty/ImGuiFileDialog/ImGuiFileDialog.cpp
)

set(IMPLOT_DIR ${CMAKE_SOURCE_DIR}/thirdparty/implot)

set(IMPLOT_SRC
    ${IMPLOT_DIR}/implot.cpp
    ${IMPLOT_DIR}/implot_items.cpp
)

set(IMGUI_TEST_ENGINE_DIR
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui_test_engine/imgui_test_engine)

set(IMGUI_TEST_ENGINE_SRC
    ${IMGUI_TEST_ENGINE_DIR}/imgui_te_engine.cpp
    ${IMGUI_TEST_ENGINE_DIR}/imgui_te_context.cpp
    ${IMGUI_TEST_ENGINE_DIR}/imgui_te_coroutine.cpp
    ${IMGUI_TEST_ENGINE_DIR}/imgui_te_ui.cpp
    ${IMGUI_TEST_ENGINE_DIR}/imgui_te_utils.cpp
    ${IMGUI_TEST_ENGINE_DIR}/imgui_te_exporters.cpp
    ${IMGUI_TEST_ENGINE_DIR}/imgui_capture_tool.cpp
    ${IMGUI_TEST_ENGINE_DIR}/imgui_te_perftool.cpp
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES 
${asset}
${functional}
${platform}
${utils}
${resource}
)

add_library(engine STATIC
${asset}
${functional}
${platform}
${utils}
${resource}
${IMGUI_SRC}
${IMPLOT_SRC}
${IMGUI_TEST_ENGINE_SRC}
)

# Fix C1041 error: allow multiple CL.EXE to write to the same PDB file
target_compile_options(engine PRIVATE /FS)

target_compile_definitions(engine PUBLIC
    IMGUI_ENABLE_TEST_ENGINE
    IMGUI_TEST_ENGINE_ENABLE_IMPLOT=1
    IMGUI_TEST_ENGINE_ENABLE_COROUTINE_STDTHREAD_IMPL=1
)

target_include_directories(engine PRIVATE
    $<TARGET_PROPERTY:volk,INTERFACE_INCLUDE_DIRECTORIES>
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/ImGuiFileDialog    
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
assimp::assimp
#spdlog::spdlog
)