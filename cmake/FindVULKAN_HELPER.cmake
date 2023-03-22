find_library(
  VULKAN_HELPER_LIBRARY
  NAMES
  vulkan_helper
  libvulkan_helper
  PATHS
  lib)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(VULKAN_HELPER REQUIRED_VARS VULKAN_HELPER_LIBRARY)
