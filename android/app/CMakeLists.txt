cmake_minimum_required(VERSION 3.6.0)
project(Small3dTests)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_library(log-lib log)
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

add_compile_definitions(SMALL3D_OPENGL)
add_compile_definitions(SMALL3D_OPENGLES)

find_package (oboe REQUIRED CONFIG)

add_library(small3d_tests
        SHARED
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/small3d_android.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/BasePath.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/BoundingBoxSet.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/File.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/GlbFile.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/Image.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/Logger.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/Model.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/Renderer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/SceneObject.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/Sound.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/Time.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include/small3d/SceneObject.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include/small3d/Sound.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include/small3d/Time.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include/small3d/BasePath.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include/small3d/BoundingBoxSet.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include/small3d/File.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include/small3d/GlbFile.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include/small3d/Image.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include/small3d/Logger.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include/small3d/Model.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include/small3d/Renderer.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/UnitTests/UnitTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/UnitTests/UnitTests.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/UnitTests/OctPyramid.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/UnitTests/OctPyramid.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/UnitTests/unit_tests.cpp
        ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
        )

target_include_directories(small3d_tests PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src # This is so that small3d_android.h can be included
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
