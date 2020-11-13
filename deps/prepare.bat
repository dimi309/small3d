set CMAKE_DEFINITIONS=-DCMAKE_BUILD_TYPE=Release 
mkdir include
mkdir lib
7z x glfw-3.3.zip
if %errorlevel% neq 0 exit /b %errorlevel%
cd glfw-3.3
mkdir build
cd build
cmake .. -G"MinGW Makefiles" -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF -DGLFW_INSTALL=OFF %CMAKE_DEFINITIONS%
cmake --build .
if %errorlevel% neq 0 exit /b %errorlevel%
xcopy ..\include\GLFW ..\..\include\GLFW /i /s
copy src\libglfw3.a ..\..\lib\
cd ..\..
rmdir /Q /S glfw-3.3

rem Only needed for OpenGL build
7z x glew-20190928.tgz
7z x glew-20190928.tar
if %errorlevel% neq 0 exit /b %errorlevel%
cd glew-2.2.0
cmake -G"MinGW Makefiles" build/cmake -DBUILD_UTILS=OFF %CMAKE_DEFINITIONS%
cmake --build .
if %errorlevel% neq 0 exit /b %errorlevel%
xcopy include\GL ..\include\GL /i /s
copy lib\libglew32.a ..\lib\
cd ..
del glew-20190928.tar
rmdir /Q /S glew-2.2.0

7z x glm-0.9.9.0.zip
if %errorlevel% neq 0 exit /b %errorlevel% 
xcopy glm\glm include\glm /i /s
rmdir /Q /S glm

7z x zlib-1.2.11.tar.gz
if %errorlevel% neq 0 exit /b %errorlevel%
7z x zlib-1.2.11.tar
cd zlib-1.2.11
mkdir build
cd build
cmake .. -G"MinGW Makefiles" %CMAKE_DEFINITIONS%
cmake --build .
if %errorlevel% neq 0 exit /b %errorlevel%
copy ..\zlib.h ..\..\include
copy zconf.h ..\..\include
copy libzlibstatic.a ..\..\lib\zlib.a
cd ..\..\
rmdir /Q /S zlib-1.2.11
del zlib-1.2.11.tar

7z x libpng-1.6.37.tar.gz
if %errorlevel% neq 0 exit /b %errorlevel%
7z x libpng-1.6.37.tar
cd libpng-1.6.37
mkdir build
cd build
cmake .. -G"MinGW Makefiles" -DPNG_SHARED=OFF -DPNG_STATIC=ON -DPNG_TESTS=OFF -DZLIB_LIBRARY=..\..\lib/zlib.a -DZLIB_INCLUDE_DIR=..\..\include %CMAKE_DEFINITIONS%
cmake --build .
if %errorlevel% neq 0 exit /b %errorlevel%
copy ..\*.h ..\..\include
copy pnglibconf.h ..\..\include
copy libpng.a ..\..\lib
cd ..\..\
rmdir /Q /S libpng-1.6.37
del libpng-1.6.37.tar
del pax_global_header

7z x ogg-1.3.3.tar.gz
if %errorlevel% neq 0 exit /b %errorlevel%
7z x ogg-1.3.3.tar
cd ogg-1.3.3
mkdir build
cd build
cmake .. -G"MinGW Makefiles" -DBUILD_SHARED_LIBS=OFF %CMAKE_DEFINITIONS%
cmake --build .
if %errorlevel% neq 0 exit /b %errorlevel%
xcopy ..\include\ogg ..\..\include\ogg /i /s
copy include\ogg\config_types.h ..\..\include\ogg
copy libogg.a ..\..\lib
cd ..\..\
rmdir /Q /S ogg-1.3.3
del ogg-1.3.3.tar
del pax_global_header

set depspath=%cd%

7z x vorbis-1.3.6.tar.gz
if %errorlevel% neq 0 exit /b %errorlevel%
7z x vorbis-1.3.6.tar
cd vorbis-1.3.6
mkdir build
cd build
cmake .. -G"MinGW Makefiles" -DBUILD_SHARED_LIBS=OFF -DOGG_ROOT=%depspath% %CMAKE_DEFINITIONS%
cmake --build .
if %errorlevel% neq 0 exit /b %errorlevel%
xcopy ..\include\vorbis ..\..\include\vorbis /i /s
copy lib\*.a ..\..\lib
cd ..\..\
rmdir /Q /S vorbis-1.3.6
del vorbis-1.3.6.tar
del pax_global_header

7z x pa_stable_v190600_20161030.tgz
if %errorlevel% neq 0 exit /b %errorlevel%
7z x pa_stable_v190600_20161030.tar
cd portaudio
mkdir build1
cd build1
cmake .. -G"MinGW Makefiles" -DPA_USE_WDMKS=OFF %CMAKE_DEFINITIONS%
cmake --build .
if %errorlevel% neq 0 exit /b %errorlevel%
copy ..\include\* ..\..\include
copy libportaudio_static.a ..\..\lib
cd ..\..\
rmdir /Q /S portaudio
del pa_stable_v190600_20161030.tar

7z x freetype-2.9.1.tar.gz
if %errorlevel% neq 0 exit /b %errorlevel%
7z x freetype-2.9.1.tar
cd freetype-2.9.1
mkdir build
cd build
cmake .. -G"MinGW Makefiles" -DBUILD_SHARED_LIBS=OFF %CMAKE_DEFINITIONS%
cmake --build .
if %errorlevel% neq 0 exit /b %errorlevel%
xcopy ..\include ..\..\include /s /e
copy libfreetype.a ..\..\lib
cd ..\..
rmdir /Q /S freetype-2.9.1
del freetype-2.9.1.tar

