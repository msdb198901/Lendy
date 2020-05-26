CollectSourceFiles(
    ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE_SOURCES)

CollectIncludeDirectories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC_INCLUDES)

add_definitions(-DLENDY_API_EXPORT_GAME)
add_library(RedBlack SHARED ${PRIVATE_SOURCES})
set_target_properties(RedBlack PROPERTIES FOLDER "SubGame") 
set_target_properties(RedBlack PROPERTIES PREFIX "")

target_include_directories(RedBlack 
  PUBLIC
    ${PUBLIC_INCLUDES}
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(RedBlack
  PUBLIC
    Net)

if( UNIX )
	add_custom_command(TARGET RedBlack
      POST_BUILD
	  COMMAND 
	  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/RedBlack.so  ${CMAKE_SOURCE_DIR}/bin/
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/RedBlack_1.ini ${CMAKE_SOURCE_DIR}/bin/)
endif()

if( UNIX )
  install(TARGETS RedBlack DESTINATION bin)
  install(FILES RedBlack_1.ini DESTINATION bin)
elseif( WIN32 )
  install(TARGETS RedBlack DESTINATION bin)
  install(FILES RedBlack_1.ini DESTINATION bin)
endif()