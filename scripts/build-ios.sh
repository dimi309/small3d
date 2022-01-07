set -e

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

cd ..
git clean -fdx

if [ "$3" != "skipdeps" ]
cd deps/scripts
./prepare-ios.sh $1 $2
cd ..
cd ..
fi

mkdir build
cd build
if [ $1 = "ios" ]
then
    cmake .. -GXcode -DCMAKE_TOOLCHAIN_FILE=../deps/ios-cmake/ios.toolchain.cmake -DPLATFORM=OS64
elif [ $1 = "simulator" ]
then
    cmake .. -GXcode -DCMAKE_TOOLCHAIN_FILE=../deps/ios-cmake/ios.toolchain.cmake -DPLATFORM=SIMULATOR64 -DARCHS=x86_64
fi

cmake --build . --config $2

mv lib/$2/* lib/

rmdir lib/$2
rm lib/interop.m

cd ../scripts

./compile-shaders.sh $2

echo "small3d built successfully for $1 ($2 mode)"
