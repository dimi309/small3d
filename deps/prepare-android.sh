# For this to work, set the NDK variable to your ndk path. It should look like
# /Users/user/Library/Android/sdk/ndk/21.2.6472646 for example.
# Tested on MacOS and Debian

mkdir include
mkdir lib

depspath=$(pwd)
platformstr=android-28

unzip glm-0.9.9.0.zip
cp -rf glm/glm include/
rm -rf glm

for androidabi in x86 x86_64 armeabi-v7a arm64-v8a
do
    mkdir lib/$androidabi
    
    tar xvf libpng-1.6.37.tar.gz
    cd libpng-1.6.37
    mkdir build
    cd build
    cmake .. -DPNG_SHARED=OFF -DPNG_STATIC=ON -DPNG_TESTS=OFF \
	  -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake -DANDROID_PLATFORM=$platformstr -DANDROID_ABI=$androidabi
    cmake --build .
    rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    cp ../*.h ../../include/
    cp pnglibconf.h ../../include/
    cp libpng.a ../../lib/$androidabi
    cd ../../
    rm -rf libpng-1.6.37

    tar xvf ogg-1.3.3.tar.gz
    cd ogg-1.3.3
    mkdir build
    cd build
    cmake .. -DBUILD_SHARED_LIBS=OFF \
	  -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake -DANDROID_PLATFORM=$platformstr -DANDROID_ABI=$androidabi
    cmake --build .
    rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    cp -rf ../include/ogg ../../include/
    cp include/ogg/config_types.h ../../include/ogg/
    cp libogg.a ../../lib/$androidabi
    cd ../../
    rm -rf ogg-1.3.3

    tar xvf vorbis-1.3.6.tar.gz
    cd vorbis-1.3.6
    mkdir build
    cd build
    cmake .. -DBUILD_SHARED_LIBS=OFF \
	  -DOGG_INCLUDE_DIRS=$depspath/include -DOGG_LIBRARIES=$depspath/lib/$andoidabi/libogg.a \
	  -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake -DANDROID_PLATFORM=$platformstr -DANDROID_ABI=$androidabi
    cmake --build .
    rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    cp -rf ../include/vorbis ../../include/
    cp lib/*.a ../../lib/$androidabi
    cd ../../
    rm -rf vorbis-1.3.6

    tar xvf freetype-2.9.1.tar.gz
    cd freetype-2.9.1
    mkdir build
    cd build
    cmake .. -DBUILD_SHARED_LIBS=OFF \
	  -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake -DANDROID_PLATFORM=$platformstr -DANDROID_ABI=$androidabi
    cmake --build .
    rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    cp -rf ../include/* ../../include/
    cp libfreetype.a ../../lib/$androidabi
    cd ../..
    rm -rf freetype-2.9.1

done




