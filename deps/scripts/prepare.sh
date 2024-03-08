set -e

cd ..

if [ "$1" != "Debug" ] && [ "$1" != "Release" ]; then
    echo "Please indicate build type: Debug or Release"
    exit 1
fi

if [ $(uname) == 'Linux' ]; then

    CMAKE_PORTAUDIO_DEFINITIONS="-DPA_USE_JACK=OFF"

    if type -p "apt" > /dev/null ; then
	sudo apt update
	# Without Install-Recommends libvulkan-dev does not get installed on travis-ci...
	sudo apt install -y -o APT::Install-Recommends=1 libgl1-mesa-dev libxinerama-dev glslang-tools libxcursor-dev libxi-dev libxrandr-dev libasound2-dev libbz2-dev
	
    elif type -p "dnf" > /dev/null ; then
	sudo dnf install -y mesa-libGL-devel
	
    elif type -p "yum" > /dev/null ; then
	sudo yum install -y mesa-libGL-devel
	
    elif type -p "pacman" > /dev/null ; then
	sudo pacman -S libxcb libfontenc libice libsm libxaw libxcomposite libxcursor libxss libxvmc mesa
	
    else
	echo "No package manager found! Cannot install preprequisites."
	exit 1
    fi
elif [ $(uname) == 'FreeBSD' ]; then
        echo "Installing jackit, using pkg and sudo (you might be required to enter your password..."
	sudo pkg install jackit
    	CMAKE_PORTAUDIO_DEFINITIONS="-DPA_USE_JACK=ON -DPA_BUILD_SHARED=OFF"
fi

CMAKE_DEFINITIONS=-DCMAKE_BUILD_TYPE=$1 

mkdir include
mkdir lib
mkdir licenses

unzip glfw-3.3.8.zip
cd glfw-3.3.8
mkdir build
cd build
cmake .. -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF -DGLFW_INSTALL=OFF $CMAKE_DEFINITIONS
cmake --build .
cp -rf ../include/GLFW ../../include/
cp src/libglfw3.a ../../lib/
cp ../LICENSE.md ../../licenses/GLFW_LICENSE
cd ../..
rm -rf glfw-3.3.8

tar xvf glew-2.2.0.tgz
cd glew-2.2.0
# Always building GLEW on *nix in Release mode because when used in Debug mode the screen remains blank.
cmake build/cmake -DBUILD_UTILS=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build .
cp -rf include/GL ../include/
# if [ "$1" == "Debug" ]; then
#    cp lib/libGLEWd.a ../lib/libGLEW.a
# else
    cp lib/libGLEW.a ../lib/libGLEW.a
# fi
cp LICENSE.txt ../licenses/GLEW_LICENSE
cd ..
rm -rf glew-2.2.0

unzip glm-0.9.9.8.zip
cp -rf glm/glm include/
cp glm/copying.txt licenses/GLM_LICENSE
rm -rf glm
tar xvf cereal-1.3.2.tar.gz
cp -rf cereal-1.3.2/include/cereal include/
rm -rf cereal-1.3.2

tar xvf zlib-1.2.11-noexample.tar.gz
cd zlib-1.2.11
mkdir build
cd build
cmake .. $CMAKE_DEFINITIONS
cmake --build .
cp ../zlib.h ../../include/
cp zconf.h ../../include/
cp libz.a ../../lib/
cp ../zlib.3.pdf ../../licenses/ZLIB_README_LICENSE.pdf
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
cp ../*.h ../../include/
cp pnglibconf.h ../../include/
cp libpng.a ../../lib/
cp ../LICENSE ../../licenses/LIBPNG_LICENSE
cd ../../
rm -rf libpng-1.6.37

tar xvf libogg-1.3.5.tar.gz
cd libogg-1.3.5
mkdir build
cd build
cmake .. -DBUILD_SHARED_LIBS=OFF $CMAKE_DEFINITIONS
cmake --build .
cp -rf ../include/ogg ../../include/
cp include/ogg/config_types.h ../../include/ogg/
cp libogg.a ../../lib/
cp ../COPYING ../../licenses/OGG_LICENSE
cd ../../
rm -rf libogg-1.3.5

tar xvf libvorbis-1.3.7.tar.gz
cd libvorbis-1.3.7
mkdir build
cd build
cmake .. -DBUILD_SHARED_LIBS=OFF -DCMAKE_PREFIX_PATH=$(pwd)/../../ $CMAKE_DEFINITIONS
cmake --build .
cp -rf ../include/vorbis ../../include/
cp lib/*.a ../../lib/
cp ../COPYING ../../licenses/VORBIS_LICENSE
cd ../../
rm -rf libvorbis-1.3.7

tar xvf pa_stable_v190700_20210406.tgz
cd portaudio
mkdir build1
cd build1
cmake .. $CMAKE_DEFINITIONS $CMAKE_PORTAUDIO_DEFINITIONS
cmake --build .
cp ../include/* ../../include/
cp libportaudio.a ../../lib/
cp ../LICENSE.txt ../../licenses/PORTAUDIO_LICENSE
cd ../../
rm -rf portaudio

tar xvf freetype-2.12.1.tar.gz
cd freetype-2.12.1
mkdir build
cd build
# Not using BrotliDec because it causes linking issues on Linux
cmake .. -DBUILD_SHARED_LIBS=OFF -DCMAKE_DISABLE_FIND_PACKAGE_BrotliDec=TRUE -DCMAKE_PREFIX_PATH=$(pwd)/../../ $CMAKE_DEFINITIONS \
      -DZLIB_LIBRARY=../../lib/zlib.a -DZLIB_INCLUDE_DIR=../../include -DFT_DISABLE_ZLIB=ON -DFT_DISABLE_HARFBUZZ=ON
cmake --build .

cp -rf ../include/* ../../include/

if [ "$1" == "Release" ]; then
    cp libfreetype.a ../../lib/
else
    cp libfreetyped.a ../../lib/libfreetype.a
fi
cp ../LICENSE.TXT ../../licenses/FREETYPE_LICENSE
cd ../..
rm -rf freetype-2.12.1

echo "small3d dependencies built successfully ($1 mode)"
