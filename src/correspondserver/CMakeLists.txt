CollectSourceFiles(
    ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE_SOURCES)

CollectIncludeDirectories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC_INCLUDES)

add_definitions(-DLENDY_API_EXPORT_GAME)

add_executable(CorrespondServer ${PRIVATE_SOURCES})

target_include_directories(CorrespondServer 
  PUBLIC
    ${PUBLIC_INCLUDES}
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(CorrespondServer 
  PUBLIC
    	Net)

if( UNIX )
	add_custom_command(TARGET CorrespondServer
      POST_BUILD
	  COMMAND 
	  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/CorrespondServer ${CMAKE_SOURCE_DIR}/bin/
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/CorrespondConfig.ini ${CMAKE_SOURCE_DIR}/bin/)
elseif( WIN32 )
	add_custom_command(TARGET CorrespondServer
      POST_BUILD
	  COMMAND
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/CorrespondConfig.ini ${CMAKE_SOURCE_DIR}/bin/)
endif()

if( UNIX )
  install(TARGETS CorrespondServer DESTINATION bin)
  install(FILES CorrespondConfig.ini DESTINATION bin)
elseif( WIN32 )
  install(TARGETS CorrespondServer DESTINATION bin)
  install(FILES CorrespondConfig.ini DESTINATION bin)
endif()