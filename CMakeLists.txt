cmake_minimum_required(VERSION 3.0.2)

project(small3d)

option(SMALL3D_OPENGL "Compile with OpenGL 3.3 / OpenGL ES" OFF)

if(IOS AND DEFINED ANDROID_PLATFORM)
  message(FATAL_ERROR "small3d cannot be built for Android and iOS at the same time (both IOS set to ON and ANDROID_PLATFORM defined)")
endif()

if(IOS)
  message(STATUS "IOS build...")
  add_compile_definitions(SMALL3D_IOS)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -x objective-c -fmodules")
endif()

message(STATUS "OpenGL/OpenGL ES: ${SMALL3D_OPENGL}")

if(SMALL3D_OPENGL)
  add_compile_definitions(SMALL3D_OPENGL)
  if(DEFINED ANDROID_PLATFORM OR IOS)
    add_compile_definitions(SMALL3D_OPENGLES)
  endif()
endif()

file(COPY "deps/include" DESTINATION ".")

file(COPY "deps/lib" DESTINATION ".")

if(MSVC)
  file(COPY "deps/bin" DESTINATION ".")
endif()

file(COPY "include" DESTINATION ".")

if(NOT DEFINED ANDROID_PLATFORM AND NOT IOS)
  file(COPY "resources" DESTINATION "./bin")
endif()
  

if(SMALL3D_OPENGL)
  file(REMOVE ${PROJECT_BINARY_DIR}/include/small3d/Renderer.hpp)
  file(COPY "opengl/include/small3d/Renderer.hpp" DESTINATION "./include/small3d")
  if(NOT DEFINED ANDROID_PLATFORM AND NOT IOS)  
    file(GLOB VULKAN_SHADERS "${PROJECT_BINARY_DIR}/bin/resources/shaders/*")
    file(REMOVE ${VULKAN_SHADERS})
    file(COPY "opengl/resources/shaders" DESTINATION "./bin/resources")
    file(COPY "opengl/resources/shaders" DESTINATION ".")
  endif()
elseif(DEFINED ANDROID_PLATFORM)
  file(COPY "src/vulkan_helper.h" DESTINATION "./include")
  file(COPY "src/small3d_android.h" DESTINATION "./include")
elseif(IOS)
  file(COPY "src/vulkan_helper.h" DESTINATION "./include")
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

if(NOT SMALL3D_OPENGL)
  if(NOT IOS) # Linking to MoltenVK explicitly on iOS
    find_package(Vulkan REQUIRED)
  endif()
endif()

if(SMALL3D_OPENGL)
  if(NOT DEFINED ANDROID_PLATFORM AND NOT IOS)
    find_package(OpenGL REQUIRED)
    find_package(GLEW REQUIRED)
  endif()
endif()

if(NOT DEFINED ANDROID_PLATFORM AND NOT IOS)
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

if(SMALL3D_OPENGL AND WIN32 AND NOT DEFINED ANDROID_PLATFORM AND NOT IOS)
  add_definitions(-DGLEW_STATIC)
endif()

add_definitions(${PNG_DEFINITIONS})

subdirs(src)
