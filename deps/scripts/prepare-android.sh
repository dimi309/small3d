# For this to work, set the NDK variable to your ndk path. It should look like
# /Users/user/Library/Android/sdk/ndk/21.2.6472646 for example.
# Tested on MacOS and Debian

cd ..

if [ "$1" != "Debug" ] && [ "$1" != "Release" ]; then
    echo "Please indicate build type: Debug or Release"
    exit 1
fi

export CMAKE_DEFINITIONS=-DCMAKE_BUILD_TYPE=$1 

mkdir include
mkdir lib

depspath=$(pwd)
platformstr=android-26

unzip glm-0.9.9.8.zip
cp -rf glm/glm include/
rm -rf glm

for androidabi in x86 x86_64 armeabi-v7a arm64-v8a
do
    mkdir lib/$androidabi

    tar xvf vkzos-0.0.1.tar.gz
    cd vkzos-0.0.1
    mkdir build
    cd build
    cmake .. -DVKZOS_TESTS=OFF \
	  -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake -DANDROID_PLATFORM=$platformstr \
	  -DANDROID_ABI=$androidabi $CMAKE_DEFINITIONS
    cmake --build .
    rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    cp -rf ../include/* ../../include/ 
    rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    cp lib/libvkzos.a ../../lib/$androidabi
    rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    cd ../..
    rm -rf vkzos-0.0.1
    
    tar xvf libpng-1.6.37.tar.gz
    cd libpng-1.6.37
    mkdir build
    cd build
    cmake .. -DPNG_SHARED=OFF -DPNG_STATIC=ON -DPNG_TESTS=OFF \
	  -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake -DANDROID_PLATFORM=$platformstr \
	  -DANDROID_ABI=$androidabi $CMAKE_DEFINITIONS
    cmake --build .
    rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    cp ../*.h ../../include/
    rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    cp pnglibconf.h ../../include/
    rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    cp libpng.a ../../lib/$androidabi
    rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    cd ../../
    rm -rf libpng-1.6.37

    tar xvf libogg-1.3.5.tar.gz
    cd libogg-1.3.5
    mkdir build
    cd build
    cmake .. -DBUILD_SHARED_LIBS=OFF \
	  -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake -DANDROID_PLATFORM=$platformstr \
	  -DANDROID_ABI=$androidabi $CMAKE_DEFINITIONS
    cmake --build .
    rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    cp -rf ../include/ogg ../../include/
    rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    cp include/ogg/config_types.h ../../include/ogg/
    rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    cp libogg.a ../../lib/$androidabi
    rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    cd ../../
    rm -rf libogg-1.3.5

    tar xvf libvorbis-1.3.7.tar.gz
    cd libvorbis-1.3.7
    mkdir build
    cd build
    cmake .. -DBUILD_SHARED_LIBS=OFF \
	  -DOGG_INCLUDE_DIRS=$depspath/include -DOGG_LIBRARIES=$depspath/lib/$andoidabi/libogg.a \
	  -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake -DANDROID_PLATFORM=$platformstr \
	  -DANDROID_ABI=$androidabi $CMAKE_DEFINITIONS
    cmake --build .
    rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    cp -rf ../include/vorbis ../../include/
    rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    cp lib/*.a ../../lib/$androidabi
    rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    cd ../../
    rm -rf libvorbis-1.3.7

    tar xvf freetype-2.11.1.tar.gz
    cd freetype-2.11.1
    mkdir build
    cd build
    cmake .. -DBUILD_SHARED_LIBS=OFF \
	  -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake -DANDROID_PLATFORM=$platformstr \
	  -DANDROID_ABI=$androidabi $CMAKE_DEFINITIONS
    cmake --build .
    rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    cp -rf ../include/* ../../include/
    rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    
    if [ "$1" == "Release" ]; then
	cp libfreetype.a ../../lib/$androidabi
    else
	cp libfreetyped.a ../../lib/$androidabi/libfreetype.a
    fi
    rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    
    cd ../..
    rm -rf freetype-2.11.1

done

echo "small3d dependencies built successfully for Android ($1 mode)"
