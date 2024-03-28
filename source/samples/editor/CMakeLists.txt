add_executable(mango_editor main.cpp)

target_link_libraries(mango_editor
    PRIVATE
    editor
    mango
    volk
    ${GLFW_LIBRARY}
    ${THREAD} 
    glslang::glslang    
    glslang::glslang-default-resource-limits
    glslang::SPIRV
    VulkanMemoryAllocator::VulkanMemoryAllocator
    spirv-cross-reflect
    spirv-cross-core
    spirv-cross-glsl
    assimp::assimp
)