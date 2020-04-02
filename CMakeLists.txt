cmake_minimum_required(VERSION 3.2)
project(lendy)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

list(APPEND CMAKE_MODULE_PATH
  "${CMAKE_SOURCE_DIR}/cmake/macros")

find_package(MySQL)

include(ConfigureScripts)
include(ConfigureBaseTargets)
include(Utils)
include(AutoCollect)
include(options)
include(showoptions)

set_property(GLOBAL PROPERTY USE_FOLDERS On)
set(CMAKE_VERBOSE_MAKEFILE ON)

find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
endif()

set(BUILDDIR ${CMAKE_BINARY_DIR})
configure_file(
    "${CMAKE_SOURCE_DIR}/vision_data.h.in.cmake"
    "${BUILDDIR}/vision_data.h"
    @ONLY
  )

add_subdirectory(apilib)
add_subdirectory(src)

if(BUILD_EXAMPLE)
add_subdirectory(tests)
endif()