CollectSourceFiles(
    ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE_SOURCES)

CollectIncludeDirectories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC_INCLUDES)

add_definitions(-DLENDY_API_EXPORT_COMMON)
add_library(DataBase SHARED ${PRIVATE_SOURCES})
set_target_properties(DataBase PROPERTIES FOLDER "KernelEngine") 

target_include_directories(DataBase
  PUBLIC
    ${PUBLIC_INCLUDES}
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(DataBase
  PUBLIC
    Log
    mysql)