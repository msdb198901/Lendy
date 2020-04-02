CollectSourceFiles(
    ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE_SOURCES)

CollectIncludeDirectories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC_INCLUDES)

add_definitions(-DLENDY_API_EXPORT_COMMON)
add_library(Log SHARED ${PRIVATE_SOURCES})
set_target_properties(Log PROPERTIES FOLDER "KernelEngine") 

target_include_directories(Log
  PUBLIC
    ${PUBLIC_INCLUDES}
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(Log
  PUBLIC
    Util)