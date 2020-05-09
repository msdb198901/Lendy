cmake_minimum_required(VERSION 3.2)
project(lendy)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

list(APPEND CMAKE_MODULE_PATH
  "${CMAKE_SOURCE_DIR}/cmake/macros")

if (WIN32)
  set(CMAKE_BUILD_TYPE "RelWithDebInfo")
else()
SET( CMAKE_CXX_FLAGS "-std=c++11 -O3")
  if( NOT CMAKE_BUILD_TYPE )
    set(CMAKE_BUILD_TYPE "Debug")
    set(CMAKE_CXX_FLAGS_DEBUG "-std=c++11 -gdwarf-2 -fPIC")
  endif()
endif()

find_package(MySQL)

include(ConfigureBaseTargets)
include(ConfigureScripts)

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