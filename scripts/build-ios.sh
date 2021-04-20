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

exit_if_error() {
    rc=$?
    if [[ $rc != 0 ]]; then
	echo "Exiting on error."
	exit $rc
    fi
}
cd ..
git clean -fdx
exit_if_error
cd deps
./prepare-ios.sh $1 $2
exit_if_error
cd ..

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
exit_if_error
rmdir lib/$2
rm lib/interop.m

echo "small3d built successfully for $1 ($2 mode)"
