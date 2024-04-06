add_executable(mango_editor main.cpp)

target_link_libraries(mango_editor
    PRIVATE    
    editor
    # engine
    # volk
    # ${GLFW_LIBRARY}
    # ${THREAD} 
    # glslang::glslang    
    # glslang::glslang-default-resource-limits
    # glslang::SPIRV
    # GPUOpen::VulkanMemoryAllocator
    # spirv-cross-reflect
    # spirv-cross-core
    # spirv-cross-glsl
    # assimp::assimp
    # spdlog::spdlog
)