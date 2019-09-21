export CMAKE_DEFINITIONS="-GXcode -DCMAKE_TOOLCHAIN_FILE=../../../../ios-cmake/ios.toolchain.cmake -DPLATFORM=OS64" 

mkdir include
mkdir lib

unzip glm-0.9.9.0.zip
cp -rf glm/glm include/
rm -rf glm

tar xvf zlib-1.2.11-noexample.tar.gz
cd zlib-1.2.11
mkdir build
cd build
cmake .. $CMAKE_DEFINITIONS
cmake --build .
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp ../zlib.h ../../include/
cp zconf.h ../../include/
cp Debug-iphoneos/libz.a ../../lib/
cd ../../
rm -rf zlib-1.2.11

tar xvf libpng-1.6.37.tar.gz
cd libpng-1.6.37
mkdir build
cd build
cmake .. -DPNG_SHARED=OFF -DPNG_STATIC=ON -DPNG_TESTS=OFF -DZLIB_LIBRARY=$(pwd)/../../lib/libza -DZLIB_INCLUDE_DIR=$(pwd)/../../include -DZ_SOLO=TRUE $CMAKE_DEFINITIONS
cmake --build .
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp ../*.h ../../include/
cp pnglibconf.h ../../include/
cp Release-iphoneos/libpng.a ../../lib/
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

tar xvf bzip2-1.0.6.tar.gz
cd bzip2-1.0.6
make
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp bzlib.h ../include/
cp libbz2.a ../lib/
cd ..
rm -rf bzip2-1.0.6

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
