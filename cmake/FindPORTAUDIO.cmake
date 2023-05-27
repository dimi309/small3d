find_path(PORTAUDIO_INCLUDE_DIRS
  NAMES
  portaudio.h
  PATHS
  include
  /usr/include
  )

find_library(
  PORTAUDIO_LIBRARIES
  NAMES
  portaudio
  PATHS
  lib
  /usr/lib
  /usr/lib/i386-linux-gnu
  )

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(PORTAUDIO REQUIRED_VARS PORTAUDIO_LIBRARIES PORTAUDIO_INCLUDE_DIRS)
