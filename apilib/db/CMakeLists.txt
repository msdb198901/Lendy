CollectSourceFiles(
    ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE_SOURCES)

CollectIncludeDirectories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC_INCLUDES)

add_definitions(-DLENDY_API_EXPORT_COMMON)
add_library(DataBase SHARED ${PRIVATE_SOURCES})
set_target_properties(DataBase PROPERTIES FOLDER "KernelEngine") 
set_target_properties(DataBase PROPERTIES PREFIX "")

target_include_directories(DataBase
  PUBLIC
    ${PUBLIC_INCLUDES}
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(DataBase
  PUBLIC
    Log
    mysql)

if( UNIX )
	add_custom_command(TARGET DataBase
      POST_BUILD
	  COMMAND 
	  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/DataBase.so ${CMAKE_SOURCE_DIR}/bin/)
endif()

if( UNIX )
  install(TARGETS DataBase DESTINATION bin)
elseif( WIN32 )
  install(TARGETS DataBase DESTINATION bin)
endif()