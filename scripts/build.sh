set -e

if [ "$1" != "Debug" ] && [ "$1" != "Release" ]; then
    echo "Please indicate build type: Debug or Release"
    exit 1
fi

CMAKE_DEFINITIONS=-DCMAKE_BUILD_TYPE=$1

cd ..
git clean -fdx

cd deps/scripts
./prepare.sh $1
cd ..

cd ..
mkdir build

cd build
cmake .. $CMAKE_DEFINITIONS
cmake --build .

cd ../scripts

echo "small3d built successfully ($1 mode)"
