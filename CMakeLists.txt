cmake_minimum_required(VERSION 3.0.2)

project(small3d)

if(IOS AND DEFINED ANDROID_PLATFORM)
  message(FATAL_ERROR "small3d cannot be built for Android and iOS at the same time (both IOS set to ON and ANDROID_PLATFORM defined)")
endif()

if(IOS)
  message(STATUS "IOS build...")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -x objective-c -fmodules")
endif()

file(COPY "deps/include" DESTINATION ".")

if(DEFINED ANDROID_PLATFORM)
  file(COPY "src/small3d_android.h" DESTINATION "./include")
endif()

file(COPY "deps/lib" DESTINATION ".")

if(MSVC)
  file(COPY "deps/bin" DESTINATION ".")
endif()

file(COPY "include" DESTINATION ".")

if(NOT DEFINED ANDROID_PLATFORM AND NOT IOS)
  file(COPY "resources" DESTINATION "./bin")
  file(COPY "resources/shaders" DESTINATION ".")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")

if(MSVC)
  add_definitions("-D_CRT_SECURE_NO_WARNINGS")
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_BINARY_DIR}/bin")
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_BINARY_DIR}/bin")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${PROJECT_BINARY_DIR}/lib")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${PROJECT_BINARY_DIR}/lib")
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${PROJECT_BINARY_DIR}/lib")
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${PROJECT_BINARY_DIR}/lib")
endif()

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(DEPS_PATH "${CMAKE_SOURCE_DIR}/deps")
set(CMAKE_PREFIX_PATH ${DEPS_PATH})

if(NOT DEFINED ANDROID_PLATFORM AND NOT IOS)
  find_package(OpenGL REQUIRED)
  find_package(GLEW REQUIRED)
  find_package(GLFW REQUIRED)
  find_package(Portaudio REQUIRED)
  find_package(PNG REQUIRED)
  find_package(GLM REQUIRED)
  find_package(OGG REQUIRED)
  find_package(VORBIS REQUIRED)
  find_package(Freetype REQUIRED)
  # on windows bzip2 is required for gcc version >= 10, otherwise a freetype associated
  # link error is produced
  if(UNIX OR (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" AND NOT
	CMAKE_CXX_COMPILER_VERSION VERSION_LESS 10))
    find_package(BZip2 REQUIRED)
  endif()
elseif(DEFINED ANDROID_PLATFORM)
  find_package(ZLIB REQUIRED)
endif()

subdirs(src)
