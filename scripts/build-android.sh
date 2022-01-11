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
    export buildtype=Release
fi
   
cd ..

sourcepath=$(pwd)
platformstr=android-26

if [ "$2" != "skipdeps" ]
then
git clean -fdx
cd deps/scripts
./prepare-android.sh $1
cd ../..
else
rm -rf build
fi

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

cd ../scripts

./compile-shaders.sh $1

if [ "$1" == "Release" ]; then
    echo "WARNING: Release builds can cause the following error on at least some devices:"
    echo "`I/Adreno: Shader compilation failed for shaderType: 0`"
fi

echo "small3d built successfully for Android ($1 mode)"
