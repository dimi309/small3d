cmake_minimum_required(VERSION 3.6.0)
project(Small3dTests)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_library(log-lib
        log)
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

file(GLOB SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../../src/*.c*)
file(GLOB HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../../src/*.h*)

add_compile_definitions(SMALL3D_OPENGL)
add_compile_definitions(SMALL3D_OPENGLES)

MESSAGE(STATUS "Sources: ${SOURCE_FILES} ${HEADER_FILES}")

find_package (oboe REQUIRED CONFIG)

add_library(small3d_tests
        SHARED
        ${SOURCE_FILES}
        ${HEADER_FILES}
        ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

if (DEFINED ANDROID_PLATFORM)
    # The following is so that small3d_android.h can be included
    target_include_directories(small3d_tests PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/../../src)
endif ()


target_include_directories(small3d_tests PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include/small3d
        ${ANDROID_NDK}/sources/android/native_app_glue
        )

target_link_libraries(small3d_tests
        android
        ${log-lib}
        GLESv2
        EGL
        oboe::oboe
        ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/lib/${ANDROID_ABI}/libpng.a
        z
        ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/lib/${ANDROID_ABI}/libvorbis.a
        ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/lib/${ANDROID_ABI}/libvorbisfile.a
        ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/lib/${ANDROID_ABI}/libogg.a
        ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/lib/${ANDROID_ABI}/libfreetype.a
        )
