CollectIncludeDirectories(
  ${CMAKE_CURRENT_SOURCE_DIR}/msgdef
  PUBLIC_INCLUDES)

add_definitions(-DASIO_STANDALONE)
add_subdirectory(correspondserver)
add_subdirectory(logonserver)
add_subdirectory(gameserver)
