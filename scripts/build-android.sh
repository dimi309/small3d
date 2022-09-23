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

if [ ! -z "$3" ] && [ "$3" != "skipdeps" ]; then
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

export opengldef=OFF
if [ "$2" == "opengles" ]; then
    export opengldef=ON
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
	  -DANDROID_PLATFORM=$platformstr -DANDROID_ABI=$androidabi -DCMAKE_BUILD_TYPE=$buildtype -DSMALL3D_OPENGL=$opengldef
    cmake --build .

    mv lib/*.a lib/$androidabi

    find . -maxdepth 1 -type f -exec rm -v {} \;
    rm -rf CMakeFiles
    rm -rf src
done


if [ "$2" != "opengles" ]; then
    cd ../scripts
    ./compile-shaders.sh $1
    echo Copying android/app/CMakeListsVulkan.txt to android/app/CMakeLists.txt
    cp ../android/app/CMakeListsVulkan.txt ../android/app/CMakeLists.txt
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
    if [ -d "android/app/src/main/assets/resources/" ]; then
	if [ ! -d "android/app/src/main/assets/resources/shaders/" ]; then
	    mkdir android/app/src/main/assets/resources/shaders/ ;
	fi
	echo "Copying shaders to build/bin/resources/shaders..."
	for f in opengl33/resources/shadersOpenGLES/* ; do
	    cp $f android/app/src/main/assets/resources/shaders/ ;
	    echo "Copied $f" ;
	done   
    fi
    echo Copying android/app/CMakeListsOpenGLES.txt to android/app/CMakeLists.txt
    cp android/app/CMakeListsOpenGLES.txt android/app/CMakeLists.txt
    
fi



echo "small3d built successfully for Android ($1 mode)"
