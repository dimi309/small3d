@echo off

cd ..

setlocal enabledelayedexpansion
set args_ok=false

if /I "%~1" == "debug" set args_ok=true
if /I "%~1" == "release" set args_ok=true

if "%args_ok%" == "false" (
echo Please indicate build type: Debug or Release
endlocal & exit /b 1
)

if /I "%~1" == "debug" set CMAKE_DEFINITIONS=-DCMAKE_BUILD_TYPE=Debug
if /I "%~1" == "release" set CMAKE_DEFINITIONS=-DCMAKE_BUILD_TYPE=Release

@echo on

mkdir include
mkdir lib
7z x glfw-3.3.8.zip
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
cd glfw-3.3.8
mkdir build
cd build
cmake .. -G"MinGW Makefiles" -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF -DGLFW_INSTALL=OFF %CMAKE_DEFINITIONS%
cmake --build .
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
xcopy ..\include\GLFW ..\..\include\GLFW /i /s
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
copy src\libglfw3.a ..\..\lib\
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
cd ..\..
rmdir /Q /S glfw-3.3.8

7z x glew-20190928.tgz
7z x glew-20190928.tar
if !errorlevel! neq 0 endlocal & exit /b !errorlevel!
cd glew-2.2.0
cmake -G"MinGW Makefiles" build/cmake -DBUILD_UTILS=OFF %CMAKE_DEFINITIONS%
cmake --build .
if !errorlevel! neq 0 endlocal & exit /b !errorlevel!
xcopy include\GL ..\include\GL /i /s
if !errorlevel! neq 0 endlocal & exit /b !errorlevel!
set GLEWLIB=libglew32.a
if /I "%~1" == "Debug" set GLEWLIB=libglew32d.a 
copy lib\!GLEWLIB! ..\lib\libglew32.a
if !errorlevel! neq 0 endlocal & exit /b !errorlevel!
cd ..
del glew-20190928.tar
rmdir /Q /S glew-2.2.0

7z x glm-0.9.9.8.zip
if %errorlevel% neq 0 endlocal & exit /b %errorlevel% 
xcopy glm\glm include\glm /i /s
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
rmdir /Q /S glm

7z x zlib-1.2.11-noexample.tar.gz
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
7z x zlib-1.2.11-noexample.tar
cd zlib-1.2.11
mkdir build
cd build
cmake .. -G"MinGW Makefiles" %CMAKE_DEFINITIONS%
cmake --build .
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
copy ..\zlib.h ..\..\include
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
copy zconf.h ..\..\include
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
copy libzlibstatic.a ..\..\lib\zlib.a
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
cd ..\..\
rmdir /Q /S zlib-1.2.11
del zlib-1.2.11-noexample.tar

7z x libpng-1.6.37.tar.gz
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
7z x libpng-1.6.37.tar
cd libpng-1.6.37
mkdir build
cd build
cmake .. -G"MinGW Makefiles" -DPNG_SHARED=OFF -DPNG_STATIC=ON -DPNG_TESTS=OFF -DZLIB_LIBRARY=..\..\lib/zlib.a -DZLIB_INCLUDE_DIR=..\..\include %CMAKE_DEFINITIONS%
cmake --build .
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
copy ..\*.h ..\..\include
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
copy pnglibconf.h ..\..\include
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
copy libpng.a ..\..\lib
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
cd ..\..\
rmdir /Q /S libpng-1.6.37
del libpng-1.6.37.tar
del pax_global_header

7z x libogg-1.3.5.tar.gz
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
7z x libogg-1.3.5.tar
cd libogg-1.3.5
mkdir build
cd build
cmake .. -G"MinGW Makefiles" -DBUILD_SHARED_LIBS=OFF %CMAKE_DEFINITIONS%
cmake --build .
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
xcopy ..\include\ogg ..\..\include\ogg /i /s
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
copy include\ogg\config_types.h ..\..\include\ogg
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
copy libogg.a ..\..\lib
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
cd ..\..\
rmdir /Q /S libogg-1.3.5
del libogg-1.3.5.tar
del pax_global_header

set depspath=%cd%

7z x libvorbis-1.3.7.tar.gz
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
7z x libvorbis-1.3.7.tar
cd libvorbis-1.3.7
mkdir build
cd build
cmake .. -G"MinGW Makefiles" -DBUILD_SHARED_LIBS=OFF -DOGG_ROOT=%depspath% %CMAKE_DEFINITIONS%
cmake --build .
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
xcopy ..\include\vorbis ..\..\include\vorbis /i /s
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
copy lib\*.a ..\..\lib
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
cd ..\..\
rmdir /Q /S libvorbis-1.3.7
del libvorbis-1.3.7.tar
del pax_global_header

7z x pa_stable_v190700_20210406.tgz
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
7z x pa_stable_v190700_20210406.tar
cd portaudio
mkdir build1
cd build1
cmake .. -G"MinGW Makefiles" -DPA_USE_WDMKS=OFF %CMAKE_DEFINITIONS%
cmake --build .
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
copy ..\include\* ..\..\include
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
copy libportaudio.a ..\..\lib
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
cd ..\..\
rmdir /Q /S portaudio
del pa_stable_v190700_20210406.tar

7z x freetype-2.12.1.tar.gz
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
7z x freetype-2.12.1.tar
cd freetype-2.12.1
mkdir build
cd build
cmake .. -G"MinGW Makefiles" -DBUILD_SHARED_LIBS=OFF %CMAKE_DEFINITIONS% -DZLIB_LIBRARY=..\..\lib/zlib.a -DZLIB_INCLUDE_DIR=..\..\include -DFT_DISABLE_ZLIB=ON
cmake --build .
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
xcopy ..\include ..\..\include /s /e
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
set FREETYPELIB=libfreetype.a
if /I "%~1" == "Debug" set FREETYPELIB=libfreetyped.a 
copy %FREETYPELIB% ..\..\lib\libfreetype.a
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
cd ..\..
rmdir /Q /S freetype-2.12.1
del freetype-2.12.1.tar

@echo small3d dependencies built successfully for MinGW (%~1 mode)

@endlocal
