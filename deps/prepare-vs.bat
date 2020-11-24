@echo off

set args_ok=false

if /I "%~1" == "Debug" set args_ok=true
if /I "%~1" == "Release" set args_ok=true

if "%args_ok%" == "false" (
echo Please indicate build type: Debug or Release
exit /B 1
)

if /I "%~1" == "Debug" set BUILDTYPE=Debug
if /I "%~1" == "Release" set BUILDTYPE=Release

@echo on

set VSCONFIG=-G"Visual Studio 16 2019" -A x64

mkdir include
mkdir lib
mkdir bin

7z x glfw-3.3.zip
if %errorlevel% neq 0 exit /b %errorlevel%
cd glfw-3.3
mkdir build
cd build
cmake .. %VSCONFIG% -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF -DGLFW_INSTALL=OFF
cmake --build . --config %BUILDTYPE%
if %errorlevel% neq 0 exit /b %errorlevel%
xcopy ..\include\GLFW ..\..\include\GLFW /i /s
copy src\%BUILDTYPE%\glfw3.lib ..\..\lib\
for /r %%a in (*.pdb) do @copy /y "%%a" ..\..\bin
cd ..\..
rmdir /Q /S glfw-3.3

rem Only needed for OpenGL build
7z x glew-20190928.tgz
7z x glew-20190928.tar
if %errorlevel% neq 0 exit /b %errorlevel%
cd glew-2.2.0
cmake %VSCONFIG% build/cmake -DBUILD_UTILS=OFF
cmake --build . --config %BUILDTYPE%
if %errorlevel% neq 0 exit /b %errorlevel%
xcopy include\GL ..\include\GL /i /s
if %BUILDTYPE%==Debug (copy lib\%BUILDTYPE%\libglew32d.lib ..\lib\glew.lib) else (copy lib\%BUILDTYPE%\libglew32.lib ..\lib\glew.lib)
for /r %%a in (*.pdb) do @copy /y "%%a" ..\bin
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
cmake .. %VSCONFIG%
cmake --build . --config %BUILDTYPE%
if %errorlevel% neq 0 exit /b %errorlevel%
copy ..\zlib.h ..\..\include
copy zconf.h ..\..\include
if %BUILDTYPE%==Debug (copy %BUILDTYPE%\zlibstaticd.lib ..\..\lib\zlib.lib) else (copy %BUILDTYPE%\zlibstatic.lib ..\..\lib\zlib.lib)
for /r %%a in (*.pdb) do @copy /y "%%a" ..\..\bin
cd ..\..\
rmdir /Q /S zlib-1.2.11
del zlib-1.2.11.tar

7z x libpng-1.6.37.tar.gz
if %errorlevel% neq 0 exit /b %errorlevel%
7z x libpng-1.6.37.tar
cd libpng-1.6.37
mkdir build
cd build
cmake .. %VSCONFIG% -DPNG_SHARED=OFF -DPNG_STATIC=ON -DPNG_TESTS=OFF -DZLIB_LIBRARY=..\..\lib/zlib.lib -DZLIB_INCLUDE_DIR=..\..\include
cmake --build . --config %BUILDTYPE%
if %errorlevel% neq 0 exit /b %errorlevel%
copy ..\*.h ..\..\include
copy pnglibconf.h ..\..\include
if %BUILDTYPE%==Debug (copy %BUILDTYPE%\libpng16_staticd.lib ..\..\lib\png.lib) else (copy %BUILDTYPE%\libpng16_static.lib ..\..\lib\png.lib)
for /r %%a in (*.pdb) do @copy /y "%%a" ..\..\bin
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
cmake .. %VSCONFIG% -DBUILD_SHARED_LIBS=OFF
cmake --build . --config %BUILDTYPE%
if %errorlevel% neq 0 exit /b %errorlevel%
xcopy ..\include\ogg ..\..\include\ogg /i /s
copy include\ogg\config_types.h ..\..\include\ogg
copy %BUILDTYPE%\ogg.lib ..\..\lib
for /r %%a in (*.pdb) do @copy /y "%%a" ..\..\bin
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
cmake .. %VSCONFIG% -DBUILD_SHARED_LIBS=OFF -DOGG_ROOT=%depspath%
cmake --build . --config %BUILDTYPE%
if %errorlevel% neq 0 exit /b %errorlevel%
xcopy ..\include\vorbis ..\..\include\vorbis /i /s
copy lib\%BUILDTYPE%\*.lib ..\..\lib
for /r %%a in (*.pdb) do @copy /y "%%a" ..\..\bin
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
cmake .. %VSCONFIG% -DPA_USE_WDMKS=OFF -DCMAKE_BUILD_TYPE=%BUILDTYPE%
cmake --build . --config %BUILDTYPE%
if %errorlevel% neq 0 exit /b %errorlevel%
copy ..\include\* ..\..\include
move %BUILDTYPE%\portaudio_static*.lib ..\..\lib\portaudio_static.lib
for /r %%a in (*.pdb) do @copy /y "%%a" ..\..\bin
cd ..\..\
rmdir /Q /S portaudio
del pa_stable_v190600_20161030.tar

7z x freetype-2.9.1.tar.gz
if %errorlevel% neq 0 exit /b %errorlevel%
7z x freetype-2.9.1.tar
cd freetype-2.9.1
mkdir build
cd build
cmake .. %VSCONFIG% -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=%BUILDTYPE%
cmake --build . --config %BUILDTYPE%
if %errorlevel% neq 0 exit /b %errorlevel%
xcopy ..\include ..\..\include /s /e
copy %BUILDTYPE%\freetype*.lib ..\..\lib
for /r %%a in (*.pdb) do @copy /y "%%a" ..\..\bin
cd ..\..
rmdir /Q /S freetype-2.9.1
del freetype-2.9.1.tar

@echo small3d dependencies built successfully for Visual Studio (%BUILDTYPE% mode)
