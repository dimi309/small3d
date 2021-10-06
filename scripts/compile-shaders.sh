set -e

if [ "$1" != "Debug" ] && [ "$1" != "Release" ]; then
    echo "Please indicate build type: Debug or Release"
    exit 1
fi

if [ "$1" == "Debug" ]; then
   export DEBUG_INFO=-g
fi

if [ "$1" == "Release" ]; then
   export DEBUG_INFO=-g0
fi

cd ../resources/shaders
echo "Compiling shaders..."

glslangValidator -V perspectiveMatrixLightedShader.vert -o perspectiveMatrixLightedShader.spv $DEBUG_INFO

glslangValidator -V textureShader.frag -o textureShader.spv $DEBUG_INFO

if [ -d "../../build/shaders/" ]; then
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
cd ../..
echo Done.
