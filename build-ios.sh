mkdir build
cd build
cmake .. -GXcode -DCMAKE_TOOLCHAIN_FILE=../deps/ios-cmake/ios.toolchain.cmake -DPLATFORM=OS64
cmake --build .
