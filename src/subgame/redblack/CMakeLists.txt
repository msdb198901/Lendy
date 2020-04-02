CollectSourceFiles(
    ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE_SOURCES)

CollectIncludeDirectories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC_INCLUDES)

add_definitions(-DLENDY_API_EXPORT_GAME)
add_library(RedBlack SHARED ${PRIVATE_SOURCES})
set_target_properties(RedBlack PROPERTIES FOLDER "SubGame") 

target_include_directories(RedBlack 
  PUBLIC
    ${PUBLIC_INCLUDES}
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR})

#target_link_libraries(RedBlack
#  PUBLIC
#    Log
#    mysql)