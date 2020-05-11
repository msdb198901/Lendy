CollectSourceFiles(
    ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE_SOURCES)

CollectIncludeDirectories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC_INCLUDES)

add_definitions(-DLENDY_API_EXPORT_COMMON)
add_library(Log SHARED ${PRIVATE_SOURCES})
set_target_properties(Log PROPERTIES FOLDER "KernelEngine") 
set_target_properties(Log PROPERTIES PREFIX "")

target_include_directories(Log
  PUBLIC
    ${PUBLIC_INCLUDES}
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(Log
  PUBLIC
    Util)

if( UNIX )
	add_custom_command(TARGET Log
      POST_BUILD
	  COMMAND 
	  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/Log.so ${CMAKE_SOURCE_DIR}/bin/)
endif()

if( UNIX )
  install(TARGETS Log DESTINATION bin)
elseif( WIN32 )
  install(TARGETS Log DESTINATION bin)
endif()