cd ..

if [ -z $VULKAN_SDK ]
then
    echo "VULKAN_SDK is not set. Please set it to your Vulkan SDK location, e.g. export VULKAN_SDK=/Users/john/Software/vulkansdk-macos-1.2.154.0/macOS"
    exit 1
fi

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
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
rm -rf glm

if [ $1 = "ios" ]
then
    export ARCH=arm64 
    export SDK=iphoneos
elif [ $1 = "simulator" ]
then
    export ARCH=x86_64
    export SDK=iphonesimulator
fi

export CHOST=aarch64-apple-darwin* # Never used arm-apple-darwin*

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
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp zconf.h ../include/
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp libz.a ../lib/
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cd ../
rm -rf zlib-1.2.11

tar xvf bzip2-1.0.8-use-env.tar.gz
cd bzip2-1.0.8
make bzip2
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp bzlib.h ../include/
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp libbz2.a ../lib/
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
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


if [ $1 = "ios" ]
then
    export CMAKE_DEFINITIONS="-GXcode -T buildsystem=1 -DCMAKE_TOOLCHAIN_FILE=../../ios-cmake/ios.toolchain.cmake -DPLATFORM=OS64"

elif [ $1 = "simulator" ]
then
    export CMAKE_DEFINITIONS="-GXcode -T buildsystem=1 -DCMAKE_TOOLCHAIN_FILE=../../ios-cmake/ios.toolchain.cmake -DPLATFORM=SIMULATOR64 -DARCHS=x86_64"
fi

tar xvf libpng-1.6.37.tar.gz
cd libpng-1.6.37
mkdir build
cd build
# Disabling PNG_ARM_NEON because on macOS arm64 it produces the error
# "PNG_ARM_NEON_FILE undefined: no support for run-time ARM NEON checks
cmake .. -DPNG_SHARED=OFF -DPNG_STATIC=ON -DPNG_TESTS=OFF -DPNG_ARM_NEON=off $CMAKE_DEFINITIONS
cmake --build . --config $2
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp ../*.h ../../include/
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp pnglibconf.h ../../include/
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp $2-$SDK/libpng.a ../../lib/
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cd ../../
rm -rf libpng-1.6.37

tar xvf ogg-1.3.3.tar.gz
cd ogg-1.3.3
mkdir build
cd build
cmake .. -DBUILD_SHARED_LIBS=OFF $CMAKE_DEFINITIONS
cmake --build . --config $2
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp -rf ../include/ogg ../../include/
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp include/ogg/config_types.h ../../include/ogg/
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp $2-$SDK/libogg.a ../../lib/
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cd ../../
rm -rf ogg-1.3.3

tar xvf vorbis-1.3.6.tar.gz
cd vorbis-1.3.6
mkdir build
cd build
cmake .. -DBUILD_SHARED_LIBS=OFF -DCMAKE_PREFIX_PATH=$(pwd)/../../ -DOGG_INCLUDE_DIRS=../../include -DOGG_LIBRARIES=../../lib/libogg.a $CMAKE_DEFINITIONS
cmake --build . --config $2
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp -rf ../include/vorbis ../../include/
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp lib/$2-$SDK/*.a ../../lib/
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cd ../../
rm -rf vorbis-1.3.6

./prepare-bzip2-ios.sh

tar xvf freetype-2.11.0.tar.gz
cd freetype-2.11.0
mkdir build
cd build
cmake .. -DBUILD_SHARED_LIBS=OFF -DCMAKE_PREFIX_PATH=$(pwd)/../../ $CMAKE_DEFINITIONS
cmake --build . --config $2
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp -rf ../include/* ../../include/
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

if [ "$2" == "Release" ]; then
    cp $2-$SDK/libfreetype.a ../../lib/
else
    cp $2-$SDK/libfreetyped.a ../../lib/libfreetype.a
fi
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

cd ../..
rm -rf freetype-2.11.0

unset $SDK

if [ $1 = "ios" ]
then
    cp $VULKAN_SDK/../MoltenVK/MoltenVK.xcframework/ios-arm64/libMoltenVK.a lib/
elif [ $1 = "simulator" ]
then
    cp $VULKAN_SDK/../MoltenVK/MoltenVK.xcframework/ios-arm64_x86_64-simulator/libMoltenVK.a lib/
fi

cp -rf $VULKAN_SDK/../MoltenVK/include/* include/
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp ios/interop.h include/
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp ios/interop.m lib/
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

echo "small3d dependencies built successfully for $1 ($2 mode)"
