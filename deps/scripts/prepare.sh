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

    if type -p "apt" > /dev/null ; then
	sudo apt update
	# Without Install-Recommends libvulkan-dev does not get installed on travis-ci...
	sudo apt install -y -o APT::Install-Recommends=1 libgl1-mesa-dev libglfw3-dev portaudio19-dev glslang-tools libvulkan-dev
	rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    elif type -p "dnf" > /dev/null ; then
	sudo dnf install -y mesa-libGL-devel glfw-devel portaudio-devel
	rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    elif type -p "yum" > /dev/null ; then
	sudo yum install -y mesa-libGL-devel glfw-devel portaudio-devel
	rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    else
	echo "No package manager found! Cannot install preprequisites."
	exit 1
    fi
fi

export CMAKE_DEFINITIONS=-DCMAKE_BUILD_TYPE=$1 

mkdir include
mkdir lib

# Not linking to GFLW statically on Linux, because on Ubuntu the needed static libraries are a mess.
if [ $(uname) != 'Linux' ]; then
    unzip glfw-3.3.2.zip
    cd glfw-3.3.2
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
    rm -rf glfw-3.3.2
fi

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
cmake .. -DPNG_SHARED=OFF -DPNG_STATIC=ON -DPNG_TESTS=OFF -DZLIB_LIBRARY=$(pwd)/../../lib/libza -DZLIB_INCLUDE_DIR=$(pwd)/../../include $CMAKE_DEFINITIONS
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

tar xvf ogg-1.3.3.tar.gz
cd ogg-1.3.3
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
rm -rf ogg-1.3.3

tar xvf vorbis-1.3.6.tar.gz
cd vorbis-1.3.6
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
rm -rf vorbis-1.3.6

# Not linking statically to portaudio on Linux, because on Ubuntu
# the needed static libraries are a mess.
if [ $(uname) != 'Linux' ]; then
    tar xvf pa_stable_v190700_20210406.tgz
    cd portaudio
    mkdir build1
    cd build1
    cmake .. $CMAKE_DEFINITIONS
    cmake --build .
    rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    cp ../include/* ../../include/
    rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    cp libportaudio.a ../../lib/
    rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    cd ../../
    rm -rf portaudio
fi

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

tar xvf freetype-2.11.0.tar.gz
cd freetype-2.11.0
mkdir build
cd build
cmake .. -DBUILD_SHARED_LIBS=OFF -DCMAKE_PREFIX_PATH=$(pwd)/../../ $CMAKE_DEFINITIONS
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
rm -rf freetype-2.11.0

echo "small3d dependencies built successfully ($1 mode)"
