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

if /I "%~1" == "debug" set BUILDTYPE=Debug
if /I "%~1" == "release" set BUILDTYPE=Release

@echo on

set VSCONFIG=-G"Visual Studio 17 2022" -A x64

mkdir include
mkdir lib
mkdir bin
mkdir licenses

7z x glfw-3.3.8.zip
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
cd glfw-3.3.8
mkdir build
cd build
cmake .. %VSCONFIG% -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF -DGLFW_INSTALL=OFF
cmake --build . --config %BUILDTYPE%
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
xcopy ..\include\GLFW ..\..\include\GLFW /i /s
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
copy src\%BUILDTYPE%\glfw3.lib ..\..\lib\
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
for /r %%a in (*.pdb) do @copy /y "%%a" ..\..\bin
copy ..\LICENSE.md ..\..\licenses\GLFW_LICENSE
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
cd ..\..
rmdir /Q /S glfw-3.3.8

7z x glew-2.2.0.tgz
7z x glew-2.2.0.tar
if !errorlevel! neq 0 endlocal & exit /b !errorlevel!
cd glew-2.2.0
cmake %VSCONFIG% build/cmake -DBUILD_UTILS=OFF
cmake --build . --config %BUILDTYPE%
if !errorlevel! neq 0 endlocal & exit /b !errorlevel!
xcopy include\GL ..\include\GL /i /s
if !errorlevel! neq 0 endlocal & exit /b !errorlevel!
if %BUILDTYPE%==Debug (copy lib\%BUILDTYPE%\libglew32d.lib ..\lib\glew.lib) else (copy lib\%BUILDTYPE%\libglew32.lib ..\lib\glew.lib)
if !errorlevel! neq 0 endlocal & exit /b !errorlevel!
for /r %%a in (*.pdb) do @copy /y "%%a" ..\bin
if !errorlevel! neq 0 endlocal & exit /b !errorlevel!
copy LICENSE.txt ..\licenses\GLEW_LICENSE
if !errorlevel! neq 0 endlocal & exit /b !errorlevel!
cd ..
del glew-2.2.0.tar
rmdir /Q /S glew-2.2.0

7z x glm-0.9.9.8.zip
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
xcopy glm\glm include\glm /i /s
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
copy glm\copying.txt licenses\GLM_LICENSE
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
rmdir /Q /S glm

7z x cereal-1.3.2.tar.gz
7z x cereal-1.3.2.tar
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
xcopy cereal-1.3.2\include\cereal include\cereal /i /s
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
copy cereal-1.3.2\LICENSE licenses\CEREAL_LICENSE
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
del 7z x cereal-1.3.2.tar
rmdir /Q /S cereal-1.3.2

7z x zlib-1.2.11-noexample.tar.gz
7z x zlib-1.2.11-noexample.tar
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
cd zlib-1.2.11
mkdir build
cd build
cmake .. %VSCONFIG%
cmake --build . --config %BUILDTYPE%
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
copy ..\zlib.h ..\..\include
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
copy zconf.h ..\..\include
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
if %BUILDTYPE%==Debug (copy %BUILDTYPE%\zlibstaticd.lib ..\..\lib\zlib.lib) else (copy %BUILDTYPE%\zlibstatic.lib ..\..\lib\zlib.lib)
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
for /r %%a in (*.pdb) do @copy /y "%%a" ..\..\bin
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
copy ..\zlib.3.pdf ..\..\licenses\ZLIB_README_LICENSE.pdf
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
cmake .. %VSCONFIG% -DPNG_SHARED=OFF -DPNG_STATIC=ON -DPNG_TESTS=OFF -DZLIB_LIBRARY=..\..\lib/zlib.lib -DZLIB_INCLUDE_DIR=..\..\include
cmake --build . --config %BUILDTYPE%
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
copy ..\*.h ..\..\include
copy pnglibconf.h ..\..\include
if %BUILDTYPE%==Debug (copy %BUILDTYPE%\libpng16_staticd.lib ..\..\lib\png.lib) else (copy %BUILDTYPE%\libpng16_static.lib ..\..\lib\png.lib)
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
for /r %%a in (*.pdb) do @copy /y "%%a" ..\..\bin
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
copy ..\LICENSE ..\..\licenses\LIBPNG_LICENSE
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
cmake .. %VSCONFIG% -DBUILD_SHARED_LIBS=OFF
cmake --build . --config %BUILDTYPE%
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
xcopy ..\include\ogg ..\..\include\ogg /i /s
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
copy include\ogg\config_types.h ..\..\include\ogg
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
copy %BUILDTYPE%\ogg.lib ..\..\lib
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
for /r %%a in (*.pdb) do @copy /y "%%a" ..\..\bin
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
copy ..\COPYING ..\..\licenses\OGG_LICENSE
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
cmake .. %VSCONFIG% -DBUILD_SHARED_LIBS=OFF -DOGG_ROOT=%depspath%
cmake --build . --config %BUILDTYPE%
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
xcopy ..\include\vorbis ..\..\include\vorbis /i /s
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
copy lib\%BUILDTYPE%\*.lib ..\..\lib
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
for /r %%a in (*.pdb) do @copy /y "%%a" ..\..\bin
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
copy ..\COPYING ..\..\licenses\VORBIS_LICENSE
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
cmake .. %VSCONFIG% -DPA_USE_WDMKS=OFF -DCMAKE_BUILD_TYPE=%BUILDTYPE%
cmake --build . --config %BUILDTYPE%
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
copy ..\include\* ..\..\include
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
move %BUILDTYPE%\portaudio_static*.lib ..\..\lib\portaudio_static.lib
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
for /r %%a in (*.pdb) do @copy /y "%%a" ..\..\bin
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
copy ..\LICENSE.txt ..\..\licenses\PORTAUDIO_LICENSE
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
cmake .. %VSCONFIG% -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=%BUILDTYPE% -DZLIB_LIBRARY=..\..\lib/zlib.lib -DZLIB_INCLUDE_DIR=..\..\include -DFT_DISABLE_ZLIB=ON
cmake --build . --config %BUILDTYPE%
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
xcopy ..\include ..\..\include /s /e
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
copy %BUILDTYPE%\freetype*.lib ..\..\lib
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
for /r %%a in (*.pdb) do @copy /y "%%a" ..\..\bin
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
copy ..\LICENSE.TXT ..\..\licenses\FREETYPE_LICENSE
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
cd ..\..
rmdir /Q /S freetype-2.12.1
del freetype-2.12.1.tar

@echo small3d dependencies built successfully for Visual Studio (%BUILDTYPE% mode)

@endlocal
