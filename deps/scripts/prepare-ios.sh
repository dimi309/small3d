set -e

cd ..

if [ -z $1 ]
then
    echo "Please indicate what we are building for, './build-ios.sh ios' for iOS devices or './build-ios.sh simulator' for the Xcode iOS Simulator."
    exit 1
else
    if [ $1 = "ios" ]
    then
	echo "Building for iOS devices..."
    elif [ $1 = "simulator" ]
    then
	echo "Building for Xcode iOS Simulator..."
    elif [ $1 = "ios32" ]
    then
	echo "Building for 32-bit iOS devices..."
    else
	echo $1 "not supported"
	exit 1
    fi    
fi

if [ "$2" != "Debug" ] && [ "$2" != "Release" ]; then
    echo "Please indicate build type: Debug or Release (second argument, e.g. ./build-ios.sh simulator Debug)"
    exit 1
fi


mkdir include
mkdir lib

unzip glm-0.9.9.8.zip
cp -rf glm/glm include/
rm -rf glm

tar xvf cereal-1.3.2.tar.gz
cp -rf cereal-1.3.2/include/cereal include/
rm -rf cereal-1.3.2

if [ $1 = "ios" ]
then
    export ARCH=arm64 
    export SDK=iphoneos
elif [ $1 = "ios32" ]
then
    export ARCH=armv7
    export SDK=iphoneos
elif [ $1 = "simulator" ]
then
    export ARCH=x86_64
    export SDK=iphonesimulator
fi

export CHOST=aarch64-apple-darwin* # Never used arm-apple-darwin*

export SDKVERSION=12 #$(xcrun --sdk $SDK --show-sdk-version) # current version
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

cp zlib.h ../include/

cp zconf.h ../include/

cp libz.a ../lib/

cd ../
rm -rf zlib-1.2.11

tar xvf bzip2-1.0.8-use-env.tar.gz
cd bzip2-1.0.8
make bzip2

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

cp ios/interop.h include/

cp ios/interop.m lib/



if [ $1 = "ios" ]
then
    CMAKE_DEFINITIONS="-GXcode -T buildsystem=12 -DCMAKE_TOOLCHAIN_FILE=../../ios-cmake/ios.toolchain.cmake -DPLATFORM=OS64"
elif [ $1 = "ios32" ]
then
    CMAKE_DEFINITIONS="-GXcode -T buildsystem=12 -DCMAKE_TOOLCHAIN_FILE=../../ios-cmake/ios.toolchain.cmake -DPLATFORM=OS -DARCHS=armv7"
elif [ $1 = "simulator" ]
then
    CMAKE_DEFINITIONS="-GXcode -T buildsystem=12 -DCMAKE_TOOLCHAIN_FILE=../../ios-cmake/ios.toolchain.cmake -DPLATFORM=SIMULATOR64 -DARCHS=x86_64"
fi

tar xvf libpng-1.6.40.tar.gz
cd libpng-1.6.40
mkdir build
cd build
# Disabling PNG_ARM_NEON because on macOS arm64 it produces the error
# "PNG_ARM_NEON_FILE undefined: no support for run-time ARM NEON checks
cmake .. -DPNG_SHARED=OFF -DPNG_STATIC=ON -DPNG_TESTS=OFF -DPNG_ARM_NEON=off $CMAKE_DEFINITIONS
cmake --build . --config $2

cp ../*.h ../../include/

cp pnglibconf.h ../../include/

cp $2-$SDK/libpng.a ../../lib/

cd ../../
rm -rf libpng-1.6.40

tar xvf libogg-1.3.5.tar.gz
cd libogg-1.3.5
mkdir build
cd build
cmake .. -DBUILD_SHARED_LIBS=OFF -DBUILD_TESTING=OFF $CMAKE_DEFINITIONS
cmake --build . --config $2

cp -rf ../include/ogg ../../include/

cp include/ogg/config_types.h ../../include/ogg/

cp $2-$SDK/libogg.a ../../lib/

cd ../../
rm -rf libogg-1.3.5

tar xvf libvorbis-1.3.7.tar.gz
cd libvorbis-1.3.7
mkdir build
cd build
cmake .. -DBUILD_SHARED_LIBS=OFF -DCMAKE_PREFIX_PATH=$(pwd)/../../ -DOGG_INCLUDE_DIR=../../include -DOGG_LIBRARY=../../lib/libogg.a $CMAKE_DEFINITIONS
cmake --build . --config $2

cp -rf ../include/vorbis ../../include/

cp lib/$2-$SDK/*.a ../../lib/

cd ../../
rm -rf libvorbis-1.3.7

tar xvf freetype-2.12.1.tar.gz
cd freetype-2.12.1
mkdir build
cd build
cmake .. -DBUILD_SHARED_LIBS=OFF -DCMAKE_PREFIX_PATH=$(pwd)/../../ $CMAKE_DEFINITIONS -DFT_DISABLE_ZLIB=ON
cmake --build . --config $2

cp -rf ../include/* ../../include/


if [ "$2" == "Release" ]; then
    cp $2-$SDK/libfreetype.a ../../lib/
else
    cp $2-$SDK/libfreetyped.a ../../lib/libfreetype.a
fi


cd ../..
rm -rf freetype-2.12.1

unset $SDK

echo "small3d dependencies built successfully for $1 ($2 mode)"
