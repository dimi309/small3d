mkdir include
mkdir lib

unzip glm-0.9.9.0.zip
cp -rf glm/glm include/
rm -rf glm

export ARCH=arm64 # armv7, arm64 or x86_64
export CHOST=aarch64-apple-darwin* # arm-apple-darwin* or aarch64-apple-darwin*
export SDK=iphoneos #iphonesimulator also possible

export SDKVERSION=9 #$(xcrun --sdk $SDK --show-sdk-version) # current version
export SDKROOT=$(xcrun --sdk $SDK --show-sdk-path) # current version
export PREFIX="/opt/$SDK-$SDKVERSION/$ARCH"

export CC=$(xcrun --sdk $SDK --find gcc)" -fembed-bitcode"
export CPP=$(xcrun --sdk $SDK --find gcc)" -E"
export CXX=$(xcrun --sdk $SDK --find g++)
export LD=$(xcrun --sdk $SDK --find ld)

export CFLAGS="$CFLAGS -arch $ARCH -isysroot $SDKROOT -I$PREFIX/include -miphoneos-version-min=$SDKVERSION"
export CPPFLAGS="$CPPFLAGS -arch $ARCH -isysroot $SDKROOT -I$PREFIX/include -miphoneos-version-min=$SDKVERSION"
export CXXFLAGS="$CXXFLAGS -arch $ARCH -isysroot $SDKROOT -I$PREFIX/include"
export LDFLAGS="$LDFLAGS -arch $ARCH -isysroot $SDKROOT -L$PREFIX/lib"
export PKG_CONFIG_PATH="$PKG_CONFIG_PATH":"$SDKROOT/usr/lib/pkgconfig":"$PREFIX/lib/pkgconfig"

tar xvf zlib-1.2.11-noexample.tar.gz
cd zlib-1.2.11
./configure
make
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp zlib.h ../include/
cp zconf.h ../include/
cp libz.a ../lib/
cd ../
rm -rf zlib-1.2.11

tar xvf bzip2-1.0.8-use-env.tar.gz
cd bzip2-1.0.8
make bzip2
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp bzlib.h ../include/
cp libbz2.a ../lib/
cd ..
rm -rf bzip2-1.0.8

unset ARCH
unset CHOST
unset SDKVERSION
unset SDKROOT
unset PREFIX
unset CC
unset CPP
unset CXX
unset LD
unset CFLAGS
unset CPPFLAGS
unset CXXFLAGS
unset LDFLAGS
unset PKG_CONFIG_PATH

export CMAKE_DEFINITIONS="-GXcode -DCMAKE_TOOLCHAIN_FILE=../../ios-cmake/ios.toolchain.cmake -DPLATFORM=OS64"

# -DPLATFORM=OS64 and no -DARCHS for arm64
# -DPLATFORM=OS -DARCHS=armv7 for armv7
# -DPLATFORM=SIMULATOR64 -DARCHS=x86_64

tar xvf libpng-1.6.37.tar.gz
cd libpng-1.6.37
mkdir build
cd build
cmake .. -DPNG_SHARED=OFF -DPNG_STATIC=ON -DPNG_TESTS=OFF -DPNG_ARM_NEON=off $CMAKE_DEFINITIONS
cmake --build .
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp ../*.h ../../include/
cp pnglibconf.h ../../include/
cp Debug-$SDK/libpng.a ../../lib/
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
cp Release-$SDK/libogg.a ../../lib/
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
cp lib/Release-$SDK/*.a ../../lib/
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
cp Release-$SDK/libfreetype.a ../../lib/
cd ../..
rm -rf freetype-2.9.1

unset $SDK
