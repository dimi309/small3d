cmake_minimum_required(VERSION 3.0.2)

project(small3d)

file(COPY "include" DESTINATION ".")
file(COPY "resources" DESTINATION "./bin")
file(COPY "deps/include" DESTINATION ".")
file(COPY "deps/lib" DESTINATION ".")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(DEPS_PATH "${CMAKE_SOURCE_DIR}/deps")
set(CMAKE_PREFIX_PATH ${DEPS_PATH})

if(WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
elseif(APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
elseif(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif(WIN32)

find_package(OpenGL REQUIRED)
find_package(GLFW REQUIRED)
find_package(GLEW REQUIRED)
find_package(PNG REQUIRED)
find_package(GLM REQUIRED)
find_package(OGG REQUIRED)
find_package(VORBIS REQUIRED)
find_package(Portaudio REQUIRED)
find_package(Freetype REQUIRED)
find_package(GTest REQUIRED)

if(UNIX)
  find_package(BZip2 REQUIRED)
endif(UNIX)

if(WIN32)
  add_definitions(-DGLEW_STATIC)
endif(WIN32)

add_definitions(${PNG_DEFINITIONS})

subdirs(src)
