if [ "$1" != "Debug" ] && [ "$1" != "Release" ]; then
    echo "Please indicate build type: Debug or Release"
    exit 1
fi

if [ ! -z "$2" ] && [ "$2" != "opengl" ]; then
    echo "The second parameter, if entered, can only be opengl if you would like to build for OpenGL."
    exit 1
fi

if [ "$2" == "opengl" ]; then
    export CMAKE_DEFINITIONS="-DCMAKE_BUILD_TYPE=$1 -DSMALL3D_OPENGL=ON"
else
    export CMAKE_DEFINITIONS=-DCMAKE_BUILD_TYPE=$1
fi

exit_if_error() {
    rc=$?
    if [[ $rc != 0 ]]; then
	echo "Exiting on error."
	exit $rc
    fi
}
cd ..
git clean -fdx
exit_if_error
cd deps
./prepare.sh $1 $2
exit_if_error
cd ..
mkdir build
cd build
cmake .. $CMAKE_DEFINITIONS
if [ $? != 0 ]; then exit $rc; fi
cmake --build .
if [ $? != 0 ]; then exit $rc; fi
echo "small3d built successfully ($1 mode)"
