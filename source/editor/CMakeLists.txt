set(TARGET_NAME editor)

#file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS "base/*.h" "asset/*.h" "simulation/*.h" editor.h)
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "base/*" "asset/*" "simulation/*" editor.cpp editor.h)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${HEADER_FILES} ${SOURCE_FILES})

add_library(${TARGET_NAME} ${HEADER_FILES} ${SOURCE_FILES})

#set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "editorf")

target_link_libraries(${TARGET_NAME} PRIVATE engine volk)

# install(TARGETS ${TARGET_NAME} DESTINATION "bin/$<$<CONFIG:Debug>:debug>$<$<CONFIG:Release>:release>")
# install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/editor.h"
#         DESTINATION "include/editor"
#   )