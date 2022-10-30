set -e

if [ "$1" != "Debug" ] && [ "$1" != "Release" ]; then
    echo "Please indicate build type: Debug or Release"
    exit 1
fi

if [ "$1" == "Debug" ]; then
   DEBUG_INFO=-g
fi

if [ "$1" == "Release" ]; then
   DEBUG_INFO=-g0
fi

cd ../resources/shaders
echo "Compiling shaders..."

glslangValidator --target-env vulkan1.0 perspectiveMatrixLightedShader.vert -o perspectiveMatrixLightedShader.spv $DEBUG_INFO

glslangValidator --target-env vulkan1.0 perspectiveMatrixLightedShaderNoJoints.vert -o perspectiveMatrixLightedShaderNoJoints.spv $DEBUG_INFO

glslangValidator --target-env vulkan1.0 textureShader.frag -o textureShader.spv $DEBUG_INFO

if [ -d "../../build/" ]; then
    if [ ! -d "../../build/shaders/" ]; then
	mkdir ../../build/shaders/ ;
    fi
    echo "Copying binaries to build/shaders..."
    for f in *.spv ; do
	cp $f ../../build/shaders/ ;
	if [ $? != 0 ]; then exit $rc; fi
	echo "Copied $f" ;
    done
fi
if [ -d "../../build/bin/resources/shaders/" ]; then
    echo "Copying binaries to build/bin/resources/shaders..."
    for f in *.spv ; do
	cp $f ../../build/bin/resources/shaders/ ;
	echo "Copied $f" ;
    done   
fi
if [ -d "../../android/app/src/main/assets/resources/" ]; then
    if [ ! -d "../../android/app/src/main/assets/resources/shaders/" ]; then
       mkdir ../../android/app/src/main/assets/resources/shaders/
    fi
    echo "Copying binaries to android/app/src/main/assets/resources/shaders..."
    for f in *.spv ; do
	cp $f ../../android/app/src/main/assets/resources/shaders/ ;
	echo "Copied $f" ;
    done   
fi
if [ -d "../../ios/small3d-Tests-ios/resources1/" ]; then
    if [ ! -d "../../ios/small3d-Tests-ios/resources1/shaders/" ]; then
	mkdir ../../ios/small3d-Tests-ios/resources1/shaders/
    fi
    echo "Copying binaries to ios/small3d-Tests-ios/resources1/shaders..."
    for f in *.spv ; do
	cp $f ../../ios/small3d-Tests-ios/resources1/shaders/ ;
	echo "Copied $f" ;
    done   
fi
cd ../..
echo Done.
