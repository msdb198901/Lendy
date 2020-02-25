# Returns the base path to the script directory in the source directory
function(WarnAboutSpacesInBuildPath)
  # Only check win32 since unix doesn't allow spaces in paths
  if (WIN32)
    string(FIND "${CMAKE_BINARY_DIR}" " " SPACE_INDEX_POS)

    if (SPACE_INDEX_POS GREATER -1)
      message("")
      message(WARNING " *** WARNING!\n"
                      " *** Your selected build directory contains spaces!\n"
                      " *** Please note that this will cause issues!")
    endif()
  endif()
endfunction()