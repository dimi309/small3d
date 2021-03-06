cmake_minimum_required(VERSION 3.0.2)

project(small3d)

option(SMALL3D_OPENGL "Compile with OpenGL 3.3" OFF)

if(IOS AND DEFINED ANDROID_PLATFORM)
  message(FATAL_ERROR "small3d cannot be built for Android and iOS at the same time (both IOS set to ON and ANDROID_PLATFORM defined)")
endif()

if(IOS)
  message(STATUS "IOS build...")
  add_compile_definitions(SMALL3D_IOS)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -x objective-c -fmodules")
endif()

message(STATUS "OpenGL: ${SMALL3D_OPENGL}")

if(SMALL3D_OPENGL)
  if(IOS OR DEFINED ANDROID_PLATFORM)
    message(FATAL_ERROR "small3d cannot be built with OpenGL for iOS or Android.")
  endif()
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

if(NOT DEFINED ANDROID_PLATFORM AND NOT IOS)
  file(COPY "resources" DESTINATION "./bin")
else()
  file(COPY "src/vkzos.h" DESTINATION "./include")
endif()

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
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
  set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
elseif(APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -stdlib=libc++")
elseif(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
endif()

if(NOT SMALL3D_OPENGL AND NOT IOS) # Linking to MoltenVK explicitly on iOS
  find_package(Vulkan REQUIRED)
endif()

if(SMALL3D_OPENGL)
  find_package(OpenGL REQUIRED)
  find_package(GLEW REQUIRED)
endif()

if(NOT DEFINED ANDROID_PLATFORM AND NOT IOS)
  find_package(GLFW REQUIRED)
  find_package(Portaudio REQUIRED)
  find_package(PNG REQUIRED)
  find_package(GLM REQUIRED)
  find_package(OGG REQUIRED)
  find_package(VORBIS REQUIRED)
  find_package(Freetype REQUIRED)
  if(UNIX)
    find_package(BZip2 REQUIRED)
  endif()
elseif(DEFINED ANDROID_PLATFORM)
  find_package(ZLIB REQUIRED)
endif()

if(SMALL3D_OPENGL AND WIN32)
  add_definitions(-DGLEW_STATIC)
endif()

add_definitions(${PNG_DEFINITIONS})

subdirs(src)
