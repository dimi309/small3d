cmake_minimum_required(VERSION 3.6.0)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
option(SMALL3D_OPENGL "Compile with OpenGL 3.3 / OpenGL ES" ON)

find_library(log-lib
        log)
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

file(GLOB SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../../src/*.c*)
file(GLOB HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../../src/*.h*)

if (SMALL3D_OPENGL)
    add_compile_definitions(SMALL3D_OPENGL)
    list(REMOVE_ITEM SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../../src/Renderer.cpp)
    list(APPEND SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../../opengl33/src/Renderer.cpp)
    list(REMOVE_ITEM HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../../include/small3d/Renderer.hpp)
    list(APPEND HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../../opengl33/include/small3d/Renderer.hpp)
    list(REMOVE_ITEM SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../../src/vulkan_helper.c)
    list(REMOVE_ITEM HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../../src/vulkan_helper.h)
endif ()

MESSAGE(STATUS "Sources: ${SOURCE_FILES} ${HEADER_FILES}")

add_library(small3d_tests
        SHARED
        ${SOURCE_FILES}
        ${HEADER_FILES}
        ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

if (NOT SMALL3D_OPENGL)
    find_package(Vulkan REQUIRED)
    message(STATUS "Vulkan FOUND = ${Vulkan_FOUND}")
    message(STATUS "Vulkan Include = ${Vulkan_INCLUDE_DIR}")
    message(STATUS "Vulkan Libs = ${Vulkan_LIBRARIES}")
endif ()

if(SMALL3D_OPENGL)
    # When using OpenGL include the OpenGL Renderer.hpp directory first
    # so that OpenGL Renderer.hpp takes precedence over Vulkan Renderer.hpp
    target_include_directories(small3d_tests PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/../../opengl33/include
            ${CMAKE_CURRENT_SOURCE_DIR}/../../opengl33/include/small3d)
    if(DEFINED ANDROID_PLATFORM)
        # The following is so that small3d_android.h can be included
        target_include_directories(small3d_tests PUBLIC
                ${CMAKE_CURRENT_SOURCE_DIR}/../../src)
    endif()
endif()

target_include_directories(small3d_tests PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include/small3d
        ${Vulkan_INCLUDE_DIRS}
        ${ANDROID_NDK}/sources/android/native_app_glue
        )

if (NOT SMALL3D_OPENGL)
    target_link_libraries(small3d_tests
            android
            aaudio
            ${log-lib}
            ${Vulkan_LIBRARIES}
            ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/lib/${ANDROID_ABI}/libpng.a
            z
            ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/lib/${ANDROID_ABI}/libvorbis.a
            ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/lib/${ANDROID_ABI}/libvorbisfile.a
            ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/lib/${ANDROID_ABI}/libogg.a
            ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/lib/${ANDROID_ABI}/libfreetype.a
            )
else ()
    target_link_libraries(small3d_tests
            android
            aaudio
            ${log-lib}
            GLESv3
            ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/lib/${ANDROID_ABI}/libpng.a
            z
            ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/lib/${ANDROID_ABI}/libvorbis.a
            ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/lib/${ANDROID_ABI}/libvorbisfile.a
            ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/lib/${ANDROID_ABI}/libogg.a
            ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/lib/${ANDROID_ABI}/libfreetype.a
            )
endif ()