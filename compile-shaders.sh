cd resources/shaders
echo "Compiling shaders..."
if [ $? != 0 ]; then exit $rc; fi
glslangValidator -V perspectiveMatrixLightedShader.vert -o perspectiveMatrixLightedShader.spv
if [ $? != 0 ]; then exit $rc; fi
glslangValidator -V textureShader.frag -o textureShader.spv
if [ $? != 0 ]; then exit $rc; fi
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
	if [ $? != 0 ]; then exit $rc; fi
	echo "Copied $f" ;
    done   
fi
cd ../..
echo Done.
