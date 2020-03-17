CollectIncludeDirectories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC_INCLUDES)

CollectSourceFiles(
    ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE_SOURCES)

list(APPEND PUBLIC_INCLUDES ${BUILDDIR}/vision_data.h)

add_definitions(-DLENDY_API_EXPORT_COMMON)
add_library(Util SHARED ${PRIVATE_SOURCES})

target_include_directories(Util
  PUBLIC
    ${PUBLIC_INCLUDES}
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(Util
  PUBLIC
    fmt
	openssl)