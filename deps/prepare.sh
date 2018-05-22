if [ $(uname) == 'Linux' ]; then

    if type -p "apt" > /dev/null ; then
	sudo apt update
	sudo apt install -y libglu1-mesa-dev libasound2-dev libjack-dev libglfw3-dev portaudio19-dev
	rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    elif type -p "dnf" > /dev/null ; then
	sudo dnf install -y mesa-libGLU-devel alsa-lib-devel jack-audio-connection-kit-devel glfw-devel portaudio-devel
	rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    elif type -p "yum" > /dev/null ; then
	sudo yum install -y mesa-libGLU-devel alsa-lib-devel jack-audio-connection-kit-devel glfw-devel portaudio-devel
	rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    else
	echo "No package manager found! Cannot install preprequisites."
	exit 1
    fi
fi

mkdir include
mkdir lib

# Not linking to GFLW statically on Linux, because on Ubuntu the needed static libraries are a mess.
if [ $(uname) != 'Linux' ]; then
    unzip glfw-master-20180409.zip
    cd glfw-master
    mkdir build
    cd build
    cmake .. -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF -DGLFW_INSTALL=OFF
    cmake --build .
    rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    cp -rf ../include/GLFW ../../include/
    cp src/libglfw3.a ../../lib/
    cd ../..
    rm -rf glfw-master
fi

unzip glew-2.1.0.zip
cd glew-2.1.0
cmake build/cmake -DBUILD_UTILS=OFF
cmake --build .
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp -rf include/GL ../include/
cp lib/libGLEW.a ../lib/
cd ..
rm -rf glew-2.1.0

unzip glm-master-20180508.zip
cp -rf glm-master/glm include/
rm -rf glm-master

tar xvf zlib-1.2.11.tar.gz
cd zlib-1.2.11
mkdir build
cd build
cmake ..
cmake --build .
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp ../zlib.h ../../include/
cp zconf.h ../../include/
cp libz.a ../../lib/
cd ../../
rm -rf zlib-1.2.11

tar xvf libpng-1.6.34.tar.gz
cd libpng-1.6.34
mkdir build
cd build
cmake .. -DPNG_SHARED=OFF -DPNG_STATIC=ON -DPNG_TESTS=OFF -DZLIB_LIBRARY=$(pwd)/../../lib/libza -DZLIB_INCLUDE_DIR=$(pwd)/../../include
cmake --build .
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp ../*.h ../../include/
cp pnglibconf.h ../../include/
cp libpng.a ../../lib/
cd ../../
rm -rf libpng-1.6.34

tar xvf googletest-release-1.8.0.tar.gz
cd googletest-release-1.8.0
mkdir build
cd build
cmake .. -DBUILD_GMOCK=OFF -DBUILD_GTEST=ON -Dgtest_disable_pthreads=ON
cmake --build .
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp -rf ../googletest/include/gtest ../../include/
cp googletest/libgtest.a ../../lib/
cp googletest/libgtest_main.a ../../lib/
cd ../..
rm -rf googletest-release-1.8.0

tar xvf ogg-1.3.3.tar.gz
cd ogg-1.3.3
mkdir build
cd build
cmake .. -DBUILD_SHARED_LIBS=OFF
cmake --build .
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp -rf ../include/ogg ../../include/
cp include/ogg/config_types.h ../../include/ogg/
cp libogg.a ../../lib/
cd ../../
rm -rf ogg-1.3.3

tar xvf vorbis-1.3.6.tar.gz
cd vorbis-1.3.6
mkdir build
cd build
cmake .. -DBUILD_SHARED_LIBS=OFF -DCMAKE_PREFIX_PATH=$(pwd)/../../
cmake --build .
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp -rf ../include/vorbis ../../include/
cp lib/*.a ../../lib/
cd ../../
rm -rf vorbis-1.3.6

# Not linking statically to portaudio on Linux, because on Ubuntu
# the needed static libraries are a mess.
if [ $(uname) != 'Linux' ]; then
    tar xvf pa_stable_v190600_20161030.tgz
    cd portaudio
    mkdir build1
    cd build1
    cmake .. -DCMAKE_BUILD_TYPE=Release
    cmake --build .
    rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    cp ../include/* ../../include/
    cp libportaudio_static.a ../../lib/
    cd ../../
    rm -rf portaudio
fi

tar xvf bzip2-1.0.6.tar.gz
cd bzip2-1.0.6
make
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp bzlib.h ../include/
cp libbz2.a ../lib/
cd ..
rm -rf bzip2-1.0.6

tar xvf freetype-2.9.tar.gz
cd freetype-2.9
mkdir build
cd build
cmake .. -DBUILD_SHARED_LIBS=OFF -DCMAKE_PREFIX_PATH=$(pwd)/../../
cmake --build .
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp -rf ../include/* ../../include/
cp libfreetype.a ../../lib/
cd ../..
rm -rf freetype-2.9
