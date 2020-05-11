CollectSourceFiles(
    ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE_SOURCES)

#CollectIncludeDirectories(
#  ${CMAKE_CURRENT_SOURCE_DIR}/netdef
#  PUBLIC_INCLUDES)

add_definitions(-DLENDY_API_EXPORT_COMMON)
add_library(Net SHARED ${PRIVATE_SOURCES})
set_target_properties(Net PROPERTIES FOLDER "KernelEngine") 
set_target_properties(Net PROPERTIES PREFIX "")

target_include_directories(Net
  PUBLIC
    ${PUBLIC_INCLUDES}
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(Net
  PUBLIC
    Log)


if( UNIX )
	add_custom_command(TARGET Net
      POST_BUILD
	  COMMAND 
	  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/Net.so ${CMAKE_SOURCE_DIR}/bin/)
endif()

if( UNIX )
  install(TARGETS Net DESTINATION bin)
elseif( WIN32 )
  install(TARGETS Net DESTINATION bin)
endif()