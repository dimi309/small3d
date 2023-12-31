set -e

if [ -z $1 ]
then
    echo "Please indicate what we are building for, './build-ios.sh ios' for iOS devices or './build-ios.sh simulator' for the Xcode iOS Simulator."
    exit 1
else
    if [ $1 = "ios" ]
    then
	echo "Building for iOS devices..."
    elif [ $1 = "simulator" ] || [ $1 = "simulatornew" ] 
    then
	echo "building for Xcode iOS Simulator..."
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

cd ..


if [ "$3" != "skipdeps" ]; then
cd deps/scripts
./prepare-ios.sh $1 $2
cd ..
cd ..
else
rm -rf build
fi

mkdir build
cd build
if [ $1 = "ios" ]
then
    cmake .. -GXcode -DCMAKE_TOOLCHAIN_FILE=../deps/ios-cmake/ios.toolchain.cmake -DPLATFORM=OS64
elif [ $1 = "ios32" ]
then
    cmake .. -GXcode -DCMAKE_TOOLCHAIN_FILE=../deps/ios-cmake/ios.toolchain.cmake -DPLATFORM=OS -DARCHS=armv7
elif [ $1 = "simulator" ]
then
    cmake .. -GXcode -DCMAKE_TOOLCHAIN_FILE=../deps/ios-cmake/ios.toolchain.cmake -DPLATFORM=SIMULATOR64 -DARCHS=x86_64
elif [ $1 = "simulatornew" ]
then
    cmake .. -GXcode -DCMAKE_TOOLCHAIN_FILE=../deps/ios-cmake/ios.toolchain.cmake -DPLATFORM=SIMULATOR64 -DARCHS=x86_64 -DDEPLOYMENT_TARGET="12.0"
fi



cmake --build . --config $2

mv lib/$2/* lib/

rmdir lib/$2

cd ..
if [ -d "build/" ]; then
    if [ ! -d "build/shaders/" ]; then
	mkdir build/shaders/ ;
    fi
    echo "Copying shaders to build/shaders..."
    for f in resources/shadersOpenGLES/* ; do
	cp $f build/shaders/ ;
	if [ $? != 0 ]; then exit $rc; fi
	echo "Copied $f" ;
    done
fi
if [ -d "ios/small3d-Tests-ios/resources1/" ]; then
    if [ ! -d "ios/small3d-Tests-ios/resources1/shaders/" ]; then
	mkdir ios/small3d-Tests-ios/resources1/shaders/ ;
    fi
    echo "Copying shaders to ios/small3d-Tests-ios/resources1/shaders..."
    for f in resources/shadersOpenGLES/* ; do
	cp $f ios/small3d-Tests-ios/resources1/shaders/ ;
	echo "Copied $f" ;
    done   
fi

echo "small3d built successfully for $1 ($2 mode)"
