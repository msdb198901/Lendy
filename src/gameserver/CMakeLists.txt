CollectSourceFiles(
    ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE_SOURCES)

CollectIncludeDirectories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC_INCLUDES)

add_definitions(-DLENDY_API_EXPORT_GAME)

add_executable(GameServer ${PRIVATE_SOURCES})

target_include_directories(GameServer 
  PUBLIC
    ${PUBLIC_INCLUDES}
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(GameServer 
  PUBLIC
	DataBase
    Net)

if( UNIX )
	add_custom_command(TARGET GameServer
      POST_BUILD
	  COMMAND 
	  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/GameServer ${CMAKE_SOURCE_DIR}/bin/
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/GameConfig.ini ${CMAKE_SOURCE_DIR}/bin/)
elseif( WIN32 )
	add_custom_command(TARGET GameServer
      POST_BUILD
	  COMMAND 
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/GameConfig.ini ${CMAKE_SOURCE_DIR}/bin/)
endif()

if( UNIX )
  install(TARGETS GameServer DESTINATION bin)
  install(FILES GameConfig.ini DESTINATION bin)
elseif( WIN32 )
  install(TARGETS GameServer DESTINATION bin)
  install(FILES GameConfig.ini DESTINATION bin)
endif()