# For this to work, set the NDK variable to your ndk path. It should look like
# /Users/user/Library/Android/sdk/ndk/21.2.6472646 for example.
# Tested on MacOS and Debian

if [ "$1" != "Debug" ] && [ "$1" != "Release" ]; then
    echo "Please indicate build type: Debug or Release"
    exit 1
fi

sourcepath=$(pwd)
platformstr=android-28

if [ -d "build" ]
then
    echo "Build directory exists!"
    exit 1
fi

mkdir build
cd build

for androidabi in x86 x86_64 armeabi-v7a arm64-v8a
do    
    cmake .. -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake \
	  -DANDROID_PLATFORM=$platformstr -DANDROID_ABI=$androidabi -DCMAKE_BUILD_TYPE=$1
    cmake --build .
    rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    mv lib/*.a lib/$androidabi
    if [ $? != 0 ]; then exit $rc; fi
    find . -maxdepth 1 -type f -exec rm -v {} \;
    rm -rf CMakeFiles
    rm -rf src
done

echo "small3d built successfully for Android ($1 mode)"
