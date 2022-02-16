find_path(VKZOS_INCLUDE_DIRS
  NAMES
  include
  PATHS
  include
  /usr/include/
  )

find_library(
  VKZOS_LIBRARIES
  NAMES
  vkzos
  libvkzos
  PATHS
  lib
  /usr/lib
  /usr/lib/i386-linux-gnu
  )

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(VKZOS REQUIRED_VARS VKZOS_LIBRARIES VKZOS_INCLUDE_DIRS)
