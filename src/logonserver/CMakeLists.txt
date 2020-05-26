CollectSourceFiles(
    ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE_SOURCES)

CollectIncludeDirectories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC_INCLUDES)

add_definitions(-DLENDY_API_EXPORT_GAME)

add_executable(LogonServer ${PRIVATE_SOURCES})

target_include_directories(LogonServer 
  PUBLIC
    ${PUBLIC_INCLUDES}
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(LogonServer 
  PUBLIC
	DataBase
    Net)

if( UNIX )
	add_custom_command(TARGET LogonServer
      POST_BUILD
	  COMMAND 
	  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/LogonServer ${CMAKE_SOURCE_DIR}/bin/
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/LogonConfig.ini ${CMAKE_SOURCE_DIR}/bin/)
endif()

if( UNIX )
  install(TARGETS LogonServer DESTINATION bin)
  install(FILES LogonConfig.ini DESTINATION bin)
elseif( WIN32 )
  install(TARGETS LogonServer DESTINATION bin)
  install(FILES LogonConfig.ini DESTINATION bin)
endif()