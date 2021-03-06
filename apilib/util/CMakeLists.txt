CollectIncludeDirectories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC_INCLUDES)

CollectSourceFiles(
    ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE_SOURCES)

list(APPEND PUBLIC_INCLUDES ${BUILDDIR}/vision_data.h)

add_definitions(-DLENDY_API_EXPORT_COMMON)
add_library(Util SHARED ${PRIVATE_SOURCES})
set_target_properties(Util PROPERTIES FOLDER "KernelEngine") 
set_target_properties(Util PROPERTIES PREFIX "")

target_include_directories(Util
  PUBLIC
    ${PUBLIC_INCLUDES}
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(Util
  PUBLIC
    fmt
	openssl
	utf8cpp)


if( UNIX )
	add_custom_command(TARGET Util
      POST_BUILD
	  COMMAND 
	  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/Util.so ${CMAKE_SOURCE_DIR}/bin/)
endif()

if( UNIX )
  install(TARGETS Util DESTINATION bin)
elseif( WIN32 )
  install(TARGETS Util DESTINATION bin)
endif()