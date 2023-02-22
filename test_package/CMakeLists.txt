cmake_minimum_required(VERSION 3.0)
project(PackageTest CXX)
find_package(small3d CONFIG REQUIRED)
add_executable(example src/example.cpp)

target_link_libraries(example small3d::small3d)
