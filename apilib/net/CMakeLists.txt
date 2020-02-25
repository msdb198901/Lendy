CollectSourceFiles(
    ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE_SOURCES)

#CollectIncludeDirectories(
#  ${CMAKE_CURRENT_SOURCE_DIR}/netdef
#  PUBLIC_INCLUDES)

add_definitions(-DLENDY_API_EXPORT_COMMON)
add_library(Net SHARED ${PRIVATE_SOURCES})

target_include_directories(Net
  PUBLIC
    ${PUBLIC_INCLUDES}
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(Net
  PUBLIC
    Log)