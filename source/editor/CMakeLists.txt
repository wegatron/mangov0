set(TARGET_NAME editor)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "base/*" "asset/*" "simulation/*" "menu/*" editor.cpp editor.h)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SOURCE_FILES})

add_library(${TARGET_NAME} ${SOURCE_FILES})

#set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "editorf")
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/thirdparty/ImGuiFileDialog)
target_link_libraries(${TARGET_NAME} PRIVATE engine volk)

# install(TARGETS ${TARGET_NAME} DESTINATION "bin/$<$<CONFIG:Debug>:debug>$<$<CONFIG:Release>:release>")
# install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/editor.h"
#         DESTINATION "include/editor"
#   )