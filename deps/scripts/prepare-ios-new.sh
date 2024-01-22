set -e

cd ..

if [ -z $1 ]
then
    echo "Please indicate what we are building for. For example './build-ios.sh ios', or './build-ios.sh iosnew / simulator / simulatornew / simulatornewarm'"
    exit 1
else
    if [ $1 = "ios" ] || [ $1 = "iosnew" ]
    then
	echo "Building for iOS devices..."
    elif [ $1 = "simulator" ] || [ $1 = "simulatornew" ] || [ $1 = "simulatornewarm" ]
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
rm -rf glm

tar xvf cereal-1.3.2.tar.gz
cp -rf cereal-1.3.2/include/cereal include/
rm -rf cereal-1.3.2

if [ $1 = "ios" ] || [ $1 = "iosnew" ] 
then
    export SDK=iphoneos
elif [ $1 = "simulator" ]
then
    export SDK=iphonesimulator
elif [ $1 = "simulatornew" ]
then
    export SDK=iphonesimulator
elif [ $1 = "simulatornewarm" ]
then
    export SDK=iphonesimulator
fi

cp ios/interop.h include/

cp ios/interop.m lib/


if [ $1 = "ios" ]
then
    CMAKE_DEFINITIONS="-GXcode -T buildsystem=1 -DCMAKE_TOOLCHAIN_FILE=../../ios-cmake/ios.toolchain.cmake -DPLATFORM=OS64"
elif [ $1 = "iosnew" ]
then
    CMAKE_DEFINITIONS="-GXcode -T buildsystem=12 -DCMAKE_TOOLCHAIN_FILE=../../ios-cmake/ios.toolchain.cmake -DPLATFORM=OS64 -DDEPLOYMENT_TARGET="12.0""
elif [ $1 = "simulator" ]
then
    CMAKE_DEFINITIONS="-GXcode -T buildsystem=1 -DCMAKE_TOOLCHAIN_FILE=../../ios-cmake/ios.toolchain.cmake -DPLATFORM=SIMULATOR64 -DARCHS=x86_64"
elif [ $1 = "simulatornew" ]
then
    CMAKE_DEFINITIONS="-GXcode -T buildsystem=12 -DCMAKE_TOOLCHAIN_FILE=../../ios-cmake/ios.toolchain.cmake -DPLATFORM=SIMULATOR64 -DARCHS=x86_64 -DDEPLOYMENT_TARGET="12.0""
elif [ $1 = "simulatornewarm" ]
then
    CMAKE_DEFINITIONS="-GXcode -T buildsystem=12 -DCMAKE_TOOLCHAIN_FILE=../../ios-cmake/ios.toolchain.cmake -DPLATFORM=SIMULATORARM64 -DDEPLOYMENT_TARGET="12.0""
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
