
list(APPEND PUBLIC_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR})

CollectIncludeDirectories(
  ${CMAKE_CURRENT_SOURCE_DIR}/define
  PUBLIC_INCLUDES)

CollectIncludeDirectories(
  ${CMAKE_CURRENT_SOURCE_DIR}/dep/asio
  PUBLIC_INCLUDES)

CollectIncludeDirectories(
  ${CMAKE_CURRENT_SOURCE_DIR}/net
  PUBLIC_INCLUDES)

CollectIncludeDirectories(
  ${CMAKE_CURRENT_SOURCE_DIR}/db
  PUBLIC_INCLUDES)

add_definitions(-DASIO_STANDALONE)
add_definitions(-DBOOST_ASIO_STANDALONE)

add_subdirectory(dep)
add_subdirectory(util)
add_subdirectory(log)
add_subdirectory(net)
add_subdirectory(db)

#add_definitions(-DLENDY_API_EXPORT_COMMON)
#add_library(KernelEngine SHARED Export.h Export.cpp)
#target_link_libraries(KernelEngine
#  PUBLIC
#    Net
#    DataBase)


