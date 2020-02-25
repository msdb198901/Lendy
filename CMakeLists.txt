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



set(CMAKE_VERBOSE_MAKEFILE ON)

find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
endif()

#CollectSourceFiles(
#    ${CMAKE_SOURCE_DIR}/src 
#    PRIVATE_SOURCES)

#CollectIncludeDirectories(
#    ${CMAKE_CURRENT_SOURCE_DIR}
#    PUBLIC_INCLUDES)

#add_library(Lenpy STATIC ${PRIVATE_SOURCES})

#target_include_directories(Lenpy
#  PUBLIC
#    ${PUBLIC_INCLUDES}
#  PRIVATE
#    ${CMAKE_CURRENT_BINARY_DIR})

#if (UNIX)
#target_link_libraries(Lenpy  
#  PRIVATE
#    lendy-core-interface
#  PUBLIC
#    boost
#    fmt
#    mysql
#    protobuf
#    )
#elseif (WIN32)
#target_link_libraries(Lenpy  
#  PRIVATE
#    lendy-core-interface
#  PUBLIC
#    ws2_32
#    )
#endif()

add_subdirectory(apilib)
add_subdirectory(src)

if(BUILD_EXAMPLE)
add_subdirectory(tests)
endif()