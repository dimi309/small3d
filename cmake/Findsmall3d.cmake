find_path(
  small3d_INCLUDE_DIR
  NAMES
  small3d
  PATHS
  include)

find_library(
  small3d_LIBRARY
  NAMES
  small3d
  libsmall3d
  PATHS
  lib)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLFW REQUIRED)
find_package(PNG REQUIRED)
find_package(GLM)
find_package(OGG REQUIRED)
find_package(VORBIS REQUIRED)
find_package(PORTAUDIO REQUIRED)
find_package(Freetype REQUIRED)

if(UNIX OR (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" AND NOT
      CMAKE_CXX_COMPILER_VERSION VERSION_LESS 10))
  find_package(BZip2 REQUIRED)
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(small3d REQUIRED_VARS small3d_LIBRARY small3d_INCLUDE_DIR)

set(small3d_LIBRARIES
  ${small3d_LIBRARY}
  ${GLEW_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${GLFW_LIBRARIES}
  ${PNG_LIBRARIES}
  ${VORBIS_LIBRARIES}
  ${OGG_LIBRARIES}
  ${PORTAUDIO_LIBRARIES}
  ${FREETYPE_LIBRARIES}
  )

set(small3d_INCLUDE_DIRS
  ${small3d_INCLUDE_DIR}
  ${GLFW_INCLUDE_DIRS}
  ${OPENGL_INCLUDE_DIR}
  ${PNG_INCLUDE_DIRS}
  ${GLM_INCLUDE_DIRS}
  ${OGG_INCLUDE_DIRS}
  ${VORBIS_INCLUDE_DIR}
  ${PORTAUDIO_INCLUDE_DIRS}
  ${FREETYPE_INCLUDE_DIRS}
  )

if(APPLE)
  list(APPEND small3d_LIBRARIES  "-framework \
              AudioUnit -framework AudioToolbox -framework CoreAudio -framework Cocoa \
              -framework IOKit -framework CoreVideo")
endif()

if(UNIX)
  set(small3d_INCLUDE_DIRS ${small3d_INCLUDE_DIRS} ${BZIP2_INCLUDE_DIRS})
  set(small3d_LIBRARIES ${small3d_LIBRARIES} ${BZIP2_LIBRARIES})
endif()

if(WIN32)
  set(small3d_LIBRARIES ${small3d_LIBRARIES} winmm)
endif()

if(UNIX AND NOT APPLE) # Linux
  set(small3d_LIBRARIES ${small3d_LIBRARIES} m pthread rt asound X11 dl)
endif()

add_library(small3d::small3d UNKNOWN IMPORTED)
set_target_properties(small3d::small3d
  PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${small3d_INCLUDE_DIRS}")

set_property(TARGET small3d::small3d
  APPEND
  PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(small3d::small3d
  PROPERTIES IMPORTED_LOCATION_RELEASE "${small3d_LIBRARY}")
set_property(TARGET small3d::small3d
  APPEND
  PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(small3d::small3d
  PROPERTIES IMPORTED_LOCATION_DEBUG "${small3d_LIBRARY}")

set_property(TARGET small3d::small3d
  PROPERTY INTERFACE_LINK_LIBRARIES
  ${small3d_LIBRARIES}
  APPEND)
