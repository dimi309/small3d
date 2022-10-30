set -e

if [ "$1" != "Debug" ] && [ "$1" != "Release" ]; then
    echo "Please indicate build type: Debug or Release"
    exit 1
fi

if [ ! -z "$2" ] && [ "$2" != "opengl" ]; then
    echo "The second parameter, if entered, can only be opengl if you would like to build for OpenGL."
    exit 1
fi

if [ "$2" == "opengl" ]; then
    CMAKE_DEFINITIONS="-DCMAKE_BUILD_TYPE=$1 -DSMALL3D_OPENGL=ON"
else
    CMAKE_DEFINITIONS=-DCMAKE_BUILD_TYPE=$1
fi

cd ..
git clean -fdx

cd deps/scripts
./prepare.sh $1 $2
cd ..

cd ..
mkdir build

cd build
cmake .. $CMAKE_DEFINITIONS
cmake --build .

cd ../scripts

if [ "$2" != "opengl" ]; then
    ./compile-shaders.sh $1;
fi

echo "small3d built successfully ($1 mode)"
