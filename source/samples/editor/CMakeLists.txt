add_executable(mango_editor
    main.cpp
    editor_tests.cpp
)

target_compile_definitions(mango_editor PRIVATE
    IMGUI_ENABLE_TEST_ENGINE
    IMGUI_TEST_ENGINE_ENABLE_COROUTINE_STDTHREAD_IMPL=1
)

target_link_libraries(mango_editor
    PRIVATE    
    editor
    volk
)