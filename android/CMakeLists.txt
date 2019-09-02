cmake_minimum_required(VERSION 3.6.0)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

add_library(native_app_glue STATIC
  ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
target_include_directories(native_app_glue PUBLIC
  ${ANDROID_NDK}/sources/android/native_app_glue)

find_library(log-lib
  log)

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

file(GLOB SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/*.cpp)
list(APPEND SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/vkzos.c)
file(GLOB HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/include/*.hpp)
list(APPEND HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/vkzos.h)

add_library(tests
  SHARED
  ${SOURCE_FILES}
  ${HEADER_FILES})

find_package(Vulkan REQUIRED)
message(STATUS "Vulkan FOUND = ${Vulkan_FOUND}")
message(STATUS "Vulkan Include = ${Vulkan_INCLUDE_DIR}")
message(STATUS "Vulkan Libs = ${Vulkan_LIBRARIES}")

find_package(ZLIB REQUIRED)

target_include_directories(tests PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/deps/include
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/include
  ${Vulkan_INCLUDE_DIRS}
  ${ZLIB_INCLUDE_DIRS}
  )

target_link_libraries(tests
  android
  native_app_glue
  aaudio
  ${log-lib}
  ${Vulkan_LIBRARIES}
  ${CMAKE_CURRENT_SOURCE_DIR}/deps/lib/${ANDROID_ABI}/libpng.a
  ${ZLIB_LIBRARIES}
  ${CMAKE_CURRENT_SOURCE_DIR}/deps/lib/${ANDROID_ABI}/libvorbis.a
  ${CMAKE_CURRENT_SOURCE_DIR}/deps/lib/${ANDROID_ABI}/libvorbisfile.a
  ${CMAKE_CURRENT_SOURCE_DIR}/deps/lib/${ANDROID_ABI}/libogg.a
  ${CMAKE_CURRENT_SOURCE_DIR}/deps/lib/${ANDROID_ABI}/libfreetype.a
  ${ZLIB_LIBRARIES})
