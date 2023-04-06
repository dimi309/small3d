# For this to work, set the NDK variable to your ndk path. It should look like
# /Users/user/Library/Android/sdk/ndk/22.1.7171670 for example.
# Because of some OpenGL ES details, an NDK version that works well is 22.1.7171670. There
# can be some glitches on newer versions.

set -e

args_ok=true

if [ "$1" != "Debug" ] && [ "$1" != "Release" ]; then
    args_ok=false
fi

if [ ! -z "$2" ] && [ "$2" != "skipdeps" ]; then
    args_ok=false
fi

if [ "$args_ok" == "false" ]; then
    echo "Please indicate build type: Debug or Release"
    exit 1
fi

if [ "$1" == "Debug" ]; then
    buildtype=Debug
else
    buildtype=Release
fi

platformstr=android-16

echo "Building on $platformstr"

cd ..

sourcepath=$(pwd)

if [ "$2" != "skipdeps" ]; then
    git clean -fdx
    cd deps/scripts
    ./prepare-android.sh $1 $platformstr
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

cd ..
if [ -d "build/" ]; then
    if [ ! -d "build/shaders/" ]; then
	mkdir build/shaders/ ;
    fi
    echo "Copying shaders to build/shaders..."
    for f in resources/shadersOpenGLES/* ; do
	cp $f build/shaders/ ;
	echo "Copied $f" ;
    done
fi
if [ -d "android/app/src/main/assets/resources/" ]; then
    if [ ! -d "android/app/src/main/assets/resources/shaders/" ]; then
	mkdir android/app/src/main/assets/resources/shaders/ ;
    fi
    echo "Copying shaders to android/app/src/main/assets/resources/shaders..."
    for f in resources/shadersOpenGLES/* ; do
	cp $f android/app/src/main/assets/resources/shaders/ ;
	echo "Copied $f" ;
    done   
fi

echo "small3d built successfully for Android ($1 mode)"
