@echo off

REM For this to work, set the %NDK% variable to your ndk path. It should look like
REM C:\Users\user\AppData\Local\Android\Sdk\ndk\22.1.7171670 for example. Also, the
REM script needs to run in an environment with MinGW set up and no settings for
REM Visual Studio.
REM Because of some OpenGL ES details, an NDK version that works well is 22.1.7171670. There
REM can be some glitches on newer versions.

cd ..

setlocal enabledelayedexpansion
set args_ok=false

if /I "%~1" == "Debug" set args_ok=true
if /I "%~1" == "Release" set args_ok=true

if "%args_ok%" == "false" (
echo Please indicate build type: Debug or Release
endlocal & exit /B 1
)

if /I "%~1" == "Debug" set CMAKE_DEFINITIONS=-DCMAKE_BUILD_TYPE=Debug
if /I "%~1" == "Release" set CMAKE_DEFINITIONS=-DCMAKE_BUILD_TYPE=Release

if /I [%2] == [] (
echo Please indicate android platform, e.g. android-26 or android-16
endlocal & exit /B 1
)

@echo on

mkdir include
mkdir lib

SET depspath=%cd%
SET platformstr=%2

7z x glm-0.9.9.8.zip
if "%errorlevel%" neq "0" endlocal & exit /b %errorlevel% 
xcopy glm\glm include\glm /i /s /y
rmdir /Q /S glm

7z x cereal-1.3.2.tar.gz
7z x cereal-1.3.2.tar
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
xcopy cereal-1.3.2\include\cereal include\cereal /i /s
del 7z x cereal-1.3.2.tar
rmdir /Q /S cereal-1.3.2
del pax_global_header

7z x oboe-1.6.1.tar.gz
if "!errorlevel!" neq "0" endlocal & exit /b !errorlevel!
7z x oboe-1.6.1.tar
if "!errorlevel!" neq "0" endlocal & exit /b !errorlevel!
xcopy oboe-1.6.1\include .\include /s /e /y
rmdir /Q /S oboe-1.6.1
del oboe-1.6.1.tar

for %%A in (x86,x86_64,armeabi-v7a,arm64-v8a) do (

mkdir lib\%%A

7z x libpng-1.6.37.tar.gz
if "!errorlevel!" neq "0" endlocal & exit /b !errorlevel!
7z x libpng-1.6.37.tar
cd libpng-1.6.37
mkdir build
cd build
cmake .. -G"MinGW Makefiles" -DPNG_SHARED=OFF -DPNG_STATIC=ON -DPNG_TESTS=OFF^
 -DCMAKE_TOOLCHAIN_FILE=%NDK%\build\cmake\android.toolchain.cmake -DANDROID_PLATFORM=%platformstr% -DANDROID_ABI=%%A^
 %CMAKE_DEFINITIONS%
cmake --build .
if "!errorlevel!" neq "0" endlocal & exit /b !errorlevel! 
copy ..\*.h ..\..\include /y
copy pnglibconf.h ..\..\include /y
if "!errorlevel!" neq "0" endlocal & exit /b !errorlevel!
copy libpng.a ..\..\lib\%%A
if "!errorlevel!" neq "0" endlocal & exit /b !errorlevel! 
cd ..\..\
rmdir /Q /S libpng-1.6.37
del libpng-1.6.37.tar
del pax_global_header

7z x libogg-1.3.5.tar.gz
if "!errorlevel!" neq "0" endlocal & exit /b !errorlevel! 
7z x libogg-1.3.5.tar
cd libogg-1.3.5
mkdir build
cd build
cmake .. -G"MinGW Makefiles" -DBUILD_SHARED_LIBS=OFF^
 -DCMAKE_TOOLCHAIN_FILE=%NDK%\build\cmake\android.toolchain.cmake -DANDROID_PLATFORM=%platformstr% -DANDROID_ABI=%%A^
 %CMAKE_DEFINITIONS%
cmake --build .
if "!errorlevel!" neq "0" endlocal & exit /b !errorlevel! 
xcopy ..\include\ogg ..\..\include\ogg /i /s /y
if "!errorlevel!" neq "0" endlocal & exit /b !errorlevel! 
copy include\ogg\config_types.h ..\..\include\ogg /y
if "!errorlevel!" neq "0" endlocal & exit /b !errorlevel! 
copy libogg.a ..\..\lib\%%A
if "!errorlevel!" neq "0" endlocal & exit /b !errorlevel! 
cd ..\..\
rmdir /Q /S libogg-1.3.5
del libogg-1.3.5.tar
del pax_global_header

7z x libvorbis-1.3.7.tar.gz
if "!errorlevel!" neq "0" endlocal & exit /b !errorlevel! 
7z x libvorbis-1.3.7.tar
cd libvorbis-1.3.7
mkdir build
cd build
cmake .. -G"MinGW Makefiles" -DBUILD_SHARED_LIBS=OFF^
 -DOGG_INCLUDE_DIR=%depspath%/include -DOGG_LIBRARY=%depspath%/lib/%%A/libogg.a^
 -DCMAKE_TOOLCHAIN_FILE=%NDK%\build\cmake\android.toolchain.cmake -DANDROID_PLATFORM=%platformstr% -DANDROID_ABI=%%A^
 %CMAKE_DEFINITIONS%
cmake --build .
if "!errorlevel!" neq "0" endlocal & exit /b !errorlevel! 
xcopy ..\include\vorbis ..\..\include\vorbis /i /s /y
if "!errorlevel!" neq "0" endlocal & exit /b !errorlevel! 
copy lib\*.a ..\..\lib\%%A
if "!errorlevel!" neq "0" endlocal & exit /b !errorlevel! 
cd ..\..\
rmdir /Q /S libvorbis-1.3.7
del libvorbis-1.3.7.tar
del pax_global_header

7z x freetype-2.12.1.tar.gz
if "!errorlevel!" neq "0" endlocal & exit /b !errorlevel! 
7z x freetype-2.12.1.tar
cd freetype-2.12.1
mkdir build
cd build
cmake .. -G"MinGW Makefiles" -DBUILD_SHARED_LIBS=OFF^
 -DCMAKE_TOOLCHAIN_FILE=%NDK%\build\cmake\android.toolchain.cmake -DANDROID_PLATFORM=%platformstr% -DANDROID_ABI=%%A^
 %CMAKE_DEFINITIONS% -DFT_DISABLE_ZLIB=ON
cmake --build .
if "!errorlevel!" neq "0" endlocal & exit /b !errorlevel! 
xcopy ..\include ..\..\include /s /e /y
if "!errorlevel!" neq "0" endlocal & exit /b !errorlevel!
set FREETYPEBIN=libfreetype.a
if /I "%~1" == "Debug" set FREETYPEBIN=libfreetyped.a
copy !FREETYPEBIN! ..\..\lib\%%A\libfreetype.a
if "!errorlevel!" neq "0" endlocal & exit /b !errorlevel! 
cd ..\..
rmdir /Q /S freetype-2.12.1
del freetype-2.12.1.tar
)

@echo small3d dependencies built successfully for Android (%~1 mode)

@endlocal
