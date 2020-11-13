cmake_minimum_required(VERSION 3.0.2)

project(small3d)

option(SMALL3D_OPENGL "Compile with OpenGL 3.3" OFF)

MESSAGE(STATUS "OpenGL: ${SMALL3D_OPENGL}")

if(SMALL3D_OPENGL)
  add_compile_definitions(SMALL3D_OPENGL)
endif()

file(COPY "deps/include" DESTINATION ".")

file(COPY "deps/lib" DESTINATION ".")

if(NOT SMALL3D_OPENGL)
  file(GLOB SHADERS "resources/shaders/*.spv")
else()
  file(GLOB SHADERS "opengl33/resources/shaders/*")
endif()

file(COPY ${SHADERS} DESTINATION "./shaders")

if(MSVC)
  file(COPY "deps/bin" DESTINATION ".")
endif()

file(COPY "include" DESTINATION ".")

file(COPY "resources" DESTINATION "./bin")

if(SMALL3D_OPENGL)
  file(REMOVE "${PROJECT_BINARY_DIR}/include/small3d/Renderer.hpp")
  file(COPY "opengl33/include/small3d/Renderer.hpp" DESTINATION "./include/small3d")
  file(GLOB VULKAN_BIN_SHADERS ${PROJECT_BINARY_DIR}/bin/resources/shaders/*)
  file(REMOVE ${VULKAN_BIN_SHADERS})
  file(COPY ${SHADERS} DESTINATION ./bin/resources/shaders) 
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

if(MSVC)
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
elseif(WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
  set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
elseif(APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
elseif(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

if(NOT SMALL3D_OPENGL)
  find_package(Vulkan REQUIRED)
else()
  find_package(OpenGL REQUIRED)
  find_package(GLEW REQUIRED)
endif()

find_package(GLFW REQUIRED)
find_package(PNG REQUIRED)
find_package(GLM REQUIRED)
find_package(OGG REQUIRED)
find_package(VORBIS REQUIRED)
find_package(Portaudio REQUIRED)
find_package(Freetype REQUIRED)

if(UNIX)
  find_package(BZip2 REQUIRED)
endif()

if(SMALL3D_OPENGL AND WIN32)
  add_definitions(-DGLEW_STATIC)
endif()

add_definitions(${PNG_DEFINITIONS})

subdirs(src)
