if [ $(uname) == 'Linux' ]; then

    if type -p "apt" > /dev/null ; then
	sudo apt update
	sudo apt install -y libgl1-mesa-dev libglfw3-dev portaudio19-dev
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

export CMAKE_DEFINITIONS=-DCMAKE_BUILD_TYPE=Release 

mkdir include
mkdir lib

# Not linking to GFLW statically on Linux, because on Ubuntu the needed static libraries are a mess.
if [ $(uname) != 'Linux' ]; then
    unzip glfw-3.3.zip
    cd glfw-3.3
    mkdir build
    cd build
    cmake .. -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF -DGLFW_INSTALL=OFF $CMAKE_DEFINITIONS
    cmake --build .
    rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    cp -rf ../include/GLFW ../../include/
    cp src/libglfw3.a ../../lib/
    cd ../..
    rm -rf glfw-3.3
fi

# Only needed for OpenGL build
tar xvf glew-20190928.tgz
cd glew-2.2.0
cmake build/cmake -DBUILD_UTILS=OFF
cmake --build .
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp -rf include/GL ../include/
cp lib/libGLEW.a ../lib/
cd ..
rm -rf glew-2.2.0

unzip glm-0.9.9.0.zip
cp -rf glm/glm include/
rm -rf glm

tar xvf zlib-1.2.11.tar.gz
cd zlib-1.2.11
mkdir build
cd build
cmake .. $CMAKE_DEFINITIONS
cmake --build .
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp ../zlib.h ../../include/
cp zconf.h ../../include/
cp libz.a ../../lib/
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
cp pnglibconf.h ../../include/
cp libpng.a ../../lib/
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
cp include/ogg/config_types.h ../../include/ogg/
cp libogg.a ../../lib/
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
cp lib/*.a ../../lib/
cd ../../
rm -rf vorbis-1.3.6

# Not linking statically to portaudio on Linux, because on Ubuntu
# the needed static libraries are a mess.
if [ $(uname) != 'Linux' ]; then
    tar xvf pa_snapshot_20201116.tgz
    cd portaudio
    mkdir build1
    cd build1
    cmake .. $CMAKE_DEFINITIONS
    cmake --build .
    rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    cp ../include/* ../../include/
    cp libportaudio.a ../../lib/
    cd ../../
    rm -rf portaudio
fi

tar xvf bzip2-1.0.8.tar.gz
cd bzip2-1.0.8
make
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp bzlib.h ../include/
cp libbz2.a ../lib/
cd ..
rm -rf bzip2-1.0.8

tar xvf freetype-2.9.1.tar.gz
cd freetype-2.9.1
mkdir build
cd build
cmake .. -DBUILD_SHARED_LIBS=OFF -DCMAKE_PREFIX_PATH=$(pwd)/../../ $CMAKE_DEFINITIONS
cmake --build .
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cp -rf ../include/* ../../include/
cp libfreetype.a ../../lib/
cd ../..
rm -rf freetype-2.9.1
