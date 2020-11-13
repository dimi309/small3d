find_path(GLEW_INCLUDE_DIRS
  NAMES
  GL
  PATHS
  include
  /usr/include/GL
  )

find_library(
  GLEW_LIBRARIES
  NAMES
  glew
  glew32
  PATHS
  lib
  /usr/lib
  /usr/lib/i386-linux-gnu
  )

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(GLEW REQUIRED_VARS GLEW_LIBRARIES GLEW_INCLUDE_DIRS)
