cmake_minimum_required(VERSION 3.0)
project(small3d)

include(conanbuildinfo.cmake)
conan_basic_setup()

if(MSVC)
  add_definitions("-D_CRT_SECURE_NO_WARNINGS")
endif()

if(SMALL3D_OPENGL)
  add_compile_definitions(SMALL3D_OPENGL)
endif()

add_subdirectory(src)
