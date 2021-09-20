# For this to work, set the NDK variable to your ndk path. It should look like
# /Users/user/Library/Android/sdk/ndk/21.2.6472646 for example.

set -e

if [ "$1" != "Debug" ] && [ "$1" != "Release" ]; then
    echo "Please indicate build type: Debug or Release"
    exit 1
fi

if [ "$1" == "Debug" ]; then
    export buildtype=Debug
else
    export buildtype=
fi
   
cd ..

sourcepath=$(pwd)
platformstr=android-26

if [ -d "build" ]
then
    echo "Build directory exists!"
    exit 1
fi

cd deps
./prepare-android.sh $1
cd ..

mkdir build
cd build

for androidabi in x86 x86_64 armeabi-v7a arm64-v8a
do    
    cmake .. -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake \
	  -DANDROID_PLATFORM=$platformstr -DANDROID_ABI=$androidabi -DCMAKE_BUILD_TYPE=$buildtype
    cmake --build .

    mv lib/*.a lib/$androidabi

    find . -maxdepth 1 -type f -exec rm -v {} \;
    rm -rf CMakeFiles
    rm -rf src
done

if [ "$1" == "Release" ]; then
    echo "Warning: Did not set cmake build type to release explicitly because that leads to the following Vulkan related error on some devices: I/Adreno: Shader compilation failed for shaderType: 0"
fi
echo "small3d built successfully for Android ($1 mode)"
