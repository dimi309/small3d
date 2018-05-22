export CMAKE_DEFINITIONS="-GXcode -DCMAKE_TOOLCHAIN_FILE=../../ios-cmake/ios.toolchain.cmake -DPLATFORM=OS64"

# -DPLATFORM=OS64 and no -DARCHS for arm64
# -DPLATFORM=OS -DARCHS=armv7 for armv7

mkdir include
mkdir lib

unzip glm-0.9.9.0.zip
cp -rf glm/glm include/
rm -rf glm

./prepare-zlib-ios.sh

tar xvf libpng-1.6.37.tar.gz
cd libpng-1.6.37
mkdir build
cd build
cmake .. -DPNG_SHARED=OFF -DPNG_STATIC=ON -DPNG_TESTS=OFF -DPNG_ARM_NEON=off $CMAKE_DEFINITIONS
cmake --build .
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp ../*.h ../../include/
cp pnglibconf.h ../../include/
cp Debug-iphoneos/libpng.a ../../lib/
cd ../../
rm -rf libpng-1.6.37

tar xvf ogg-1.3.3.tar.gz
cd ogg-1.3.3
mkdir build
cd build
cmake .. -DBUILD_SHARED_LIBS=OFF $CMAKE_DEFINITIONS
cmake --build . --config Release
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp -rf ../include/ogg ../../include/
cp include/ogg/config_types.h ../../include/ogg/
cp Release-iphoneos/libogg.a ../../lib/
cd ../../
rm -rf ogg-1.3.3

tar xvf vorbis-1.3.6.tar.gz
cd vorbis-1.3.6
mkdir build
cd build
cmake .. -DBUILD_SHARED_LIBS=OFF -DCMAKE_PREFIX_PATH=$(pwd)/../../ -DOGG_INCLUDE_DIRS=../../include -DOGG_LIBRARIES=../../lib/libogg.a $CMAKE_DEFINITIONS
cmake --build . --config Release
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp -rf ../include/vorbis ../../include/
cp lib/Release-iphoneos/*.a ../../lib/
cd ../../
rm -rf vorbis-1.3.6

./prepare-bzip2-ios.sh

tar xvf freetype-2.9.1.tar.gz
cd freetype-2.9.1
mkdir build
cd build
cmake .. -DBUILD_SHARED_LIBS=OFF -DCMAKE_PREFIX_PATH=$(pwd)/../../ $CMAKE_DEFINITIONS
cmake --build . --config Release
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp -rf ../include/* ../../include/
cp Release-iphoneos/libfreetype.a ../../lib/
cd ../..
rm -rf freetype-2.9.1
