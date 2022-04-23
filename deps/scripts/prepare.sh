cd ..

if [ "$1" != "Debug" ] && [ "$1" != "Release" ]; then
    echo "Please indicate build type: Debug or Release"
    exit 1
fi

if [ ! -z "$2" ] && [ "$2" != "opengl" ]; then
    echo "The second parameter, if entered, can only be opengl if you would like to prepare the OpenGL-related dependencies"
    exit 1
fi



if [ $(uname) == 'Linux' ]; then

    export CMAKE_PORTAUDIO_DEFINITIONS="-DPA_USE_JACK=OFF"

    if type -p "apt" > /dev/null ; then
	sudo apt update
	# Without Install-Recommends libvulkan-dev does not get installed on travis-ci...
	sudo apt install -y -o APT::Install-Recommends=1 libgl1-mesa-dev libxinerama-dev glslang-tools libvulkan-dev libxcursor-dev libxi-dev
	rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    elif type -p "dnf" > /dev/null ; then
	sudo dnf install -y mesa-libGL-devel
	rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    elif type -p "yum" > /dev/null ; then
	sudo yum install -y mesa-libGL-devel
	rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    else
	echo "No package manager found! Cannot install preprequisites."
	exit 1
    fi
fi

export CMAKE_DEFINITIONS=-DCMAKE_BUILD_TYPE=$1 

mkdir include
mkdir lib


unzip glfw-3.3.7.zip
cd glfw-3.3.7
mkdir build
cd build
cmake .. -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF -DGLFW_INSTALL=OFF $CMAKE_DEFINITIONS
cmake --build .
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp -rf ../include/GLFW ../../include/
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp src/libglfw3.a ../../lib/
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cd ../..
rm -rf glfw-3.3.7

# Only needed for OpenGL build
if [ "$2" == "opengl" ]; then
    tar xvf glew-20190928.tgz
    cd glew-2.2.0
    cmake build/cmake -DBUILD_UTILS=OFF
    cmake --build .
    rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    cp -rf include/GL ../include/
    rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    cp lib/libGLEW.a ../lib/
    rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    cd ..
    rm -rf glew-2.2.0
fi

unzip glm-0.9.9.8.zip
cp -rf glm/glm include/
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
rm -rf glm

tar xvf zlib-1.2.11-noexample.tar.gz
cd zlib-1.2.11
mkdir build
cd build
cmake .. $CMAKE_DEFINITIONS
cmake --build .
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp ../zlib.h ../../include/
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp zconf.h ../../include/
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp libz.a ../../lib/
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cd ../../
rm -rf zlib-1.2.11

tar xvf libpng-1.6.37.tar.gz
cd libpng-1.6.37
mkdir build
cd build
# Disabling PNG_ARM_NEON because on macOS arm64 it produces the error
# "PNG_ARM_NEON_FILE undefined: no support for run-time ARM NEON checks
cmake .. -DPNG_SHARED=OFF -DPNG_STATIC=ON -DPNG_TESTS=OFF -DPNG_ARM_NEON=off -DZLIB_LIBRARY=$(pwd)/../../lib/libza -DZLIB_INCLUDE_DIR=$(pwd)/../../include $CMAKE_DEFINITIONS
cmake --build .
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp ../*.h ../../include/
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp pnglibconf.h ../../include/
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp libpng.a ../../lib/
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cd ../../
rm -rf libpng-1.6.37

tar xvf libogg-1.3.5.tar.gz
cd libogg-1.3.5
mkdir build
cd build
cmake .. -DBUILD_SHARED_LIBS=OFF $CMAKE_DEFINITIONS
cmake --build .
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp -rf ../include/ogg ../../include/
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp include/ogg/config_types.h ../../include/ogg/
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp libogg.a ../../lib/
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cd ../../
rm -rf libogg-1.3.5

tar xvf libvorbis-1.3.7.tar.gz
cd libvorbis-1.3.7
mkdir build
cd build
cmake .. -DBUILD_SHARED_LIBS=OFF -DCMAKE_PREFIX_PATH=$(pwd)/../../ $CMAKE_DEFINITIONS
cmake --build .
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp -rf ../include/vorbis ../../include/
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp lib/*.a ../../lib/
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cd ../../
rm -rf libvorbis-1.3.7

tar xvf pa_stable_v190700_20210406.tgz
cd portaudio
mkdir build1
cd build1
cmake .. $CMAKE_DEFINITIONS $CMAKE_PORTAUDIO_DEFINITIONS
cmake --build .
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp ../include/* ../../include/
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp libportaudio.a ../../lib/
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cd ../../
rm -rf portaudio

tar xvf bzip2-1.0.8-use-env.tar.gz
cd bzip2-1.0.8
make
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp bzlib.h ../include/
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp libbz2.a ../lib/
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cd ..
rm -rf bzip2-1.0.8

tar xvf freetype-2.11.1.tar.gz
cd freetype-2.11.1
mkdir build
cd build
# Not using BrotliDec because it causes linking issues on Linux
cmake .. -DBUILD_SHARED_LIBS=OFF -DCMAKE_DISABLE_FIND_PACKAGE_BrotliDec=TRUE -DCMAKE_PREFIX_PATH=$(pwd)/../../ $CMAKE_DEFINITIONS
cmake --build .
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp -rf ../include/* ../../include/
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
if [ "$1" == "Release" ]; then
    cp libfreetype.a ../../lib/
else
    cp libfreetyped.a ../../lib/libfreetype.a
fi
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cd ../..
rm -rf freetype-2.11.1

echo "small3d dependencies built successfully ($1 mode)"
