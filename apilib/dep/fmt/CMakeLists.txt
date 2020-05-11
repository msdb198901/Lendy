# Copyright (C) 2008-2019 TrinityCore <https://www.trinitycore.org/>
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
include(CheckSymbolExists)
if (WIN32)
  check_symbol_exists(open io.h HAVE_OPEN)
else ()
  check_symbol_exists(open fcntl.h HAVE_OPEN)
endif ()

set(FMT_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/fmt/container.h
  ${CMAKE_CURRENT_SOURCE_DIR}/fmt/format.h
  ${CMAKE_CURRENT_SOURCE_DIR}/fmt/format.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/fmt/ostream.h
  ${CMAKE_CURRENT_SOURCE_DIR}/fmt/ostream.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/fmt/printf.h
  ${CMAKE_CURRENT_SOURCE_DIR}/fmt/printf.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/fmt/string.h
  ${CMAKE_CURRENT_SOURCE_DIR}/fmt/time.h)

if (HAVE_OPEN)
  set(FMT_SOURCES ${FMT_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/fmt/posix.h
    ${CMAKE_CURRENT_SOURCE_DIR}/fmt/posix.cc)
endif()

add_library(fmt STATIC ${FMT_SOURCES})
set_target_properties(fmt PROPERTIES FOLDER "KernelEngine") 
set_target_properties(fmt PROPERTIES PREFIX "")

target_include_directories(fmt
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(fmt
  PUBLIC
    -DFMT_USE_OVERRIDE
    -DFMT_USE_VARIADIC_TEMPLATES
    -DFMT_USE_RVALUE_REFERENCES
    -DFMT_USE_DELETED_FUNCTIONS
    -DFMT_USE_EXTERN_TEMPLATES)

target_link_libraries(fmt
  PRIVATE
    lendy-dependency-interface)

if( UNIX )
	add_custom_command(TARGET fmt
      POST_BUILD
	  COMMAND 
	  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/fmt.a ${CMAKE_SOURCE_DIR}/bin/)
endif()

if( UNIX )
  install(TARGETS fmt DESTINATION bin)
elseif( WIN32 )
  install(TARGETS fmt DESTINATION bin)
endif()
