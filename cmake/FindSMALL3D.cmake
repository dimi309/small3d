find_path(
  SMALL3D_INCLUDE_DIR
  NAMES
  small3d
  PATHS
  include)

find_library(
  SMALL3D_LIBRARY
  NAMES
  small3d
  libsmall3d
  PATHS
  lib)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(SMALL3D REQUIRED_VARS SMALL3D_LIBRARY SMALL3D_INCLUDE_DIR)
