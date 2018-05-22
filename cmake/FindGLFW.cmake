find_path(GLFW_INCLUDE_DIRS
  NAMES
  GLFW
  PATHS
  include
  /usr/include/GL
  )

find_library(
  GLFW_LIBRARIES
  NAMES
  glfw3
  glfw
  PATHS
  lib
  /usr/lib
  /usr/lib/i386-linux-gnu
  )

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(GLFW REQUIRED_VARS GLFW_LIBRARIES GLFW_INCLUDE_DIRS)
