# For this to work, set the NDK variable to your ndk path. It should look like
# /Users/user/Library/Android/sdk/ndk/21.2.6472646 for example.

set -e

args_ok=true

if [ "$1" != "Debug" ] && [ "$1" != "Release" ]; then
    args_ok=false
fi

if [ ! -z "$2" ] && [ "$2" != "opengles" ] && [ "$2" != "skipdeps" ]; then
    args_ok=false
fi

if [ ! -z "$3" ] && [ "$2" != "skipdeps" ]; then
    args_ok=false
fi

if [ "$args_ok" == "false" ]; then
    echo "Please indicate build type: Debug or Release, followed by opengles if you would like to build in OpenGL ES based mode."
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

if [ "$2" != "skipdeps" ] && [ "$3" != "skipdeps" ]; then
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


if [ "$2" != "opengles" ]; then
    cd ../scripts
    ./compile-shaders.sh $1
else
    cd ..
    if [ -d "build/" ]; then
	if [ ! -d "build/shaders/" ]; then
	    mkdir build/shaders/ ;
	fi
	echo "Copying shaders to build/shaders..."
	for f in opengl33/resources/shadersOpenGLES/* ; do
	    cp $f build/shaders/ ;
	    if [ $? != 0 ]; then exit $rc; fi
	    echo "Copied $f" ;
	done
    fi
    if [ -d "android/app/src/main/assets/resources/shaders/" ]; then
	echo "Copying shaders to build/bin/resources/shaders..."
	for f in opengl33/resources/shadersOpenGLES/* ; do
	    cp $f android/app/src/main/assets/resources/shaders/ ;
	    echo "Copied $f" ;
	done   
    fi
    
fi



echo "small3d built successfully for Android ($1 mode)"
