@echo off

REM For this to work, set the %NDK% variable to your ndk path. It should look like
REM C:\Users\user\AppData\Local\Android\Sdk\ndk\22.0.6917172 for example. Also, the
REM script needs to run in an environment with MinGW set up and no settings for
REM Visual Studio.

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

@echo on

mkdir include
mkdir lib

SET depspath=%cd%
SET platformstr=android-26

7z x glm-0.9.9.8.zip
if "%errorlevel%" neq "0" endlocal & exit /b %errorlevel% 
xcopy glm\glm include\glm /i /s /y
rmdir /Q /S glm

for %%A in (x86,x86_64,armeabi-v7a,arm64-v8a) do (

mkdir lib\%%A

7z x vkzos-0.0.1.tar.gz
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
7z x vkzos-0.0.1.tar
cd vkzos-0.0.1
mkdir build
cd build
cmake .. -G"MinGW Makefiles" -DVKZOS_TESTS=OFF^
 -DCMAKE_TOOLCHAIN_FILE=%NDK%\build\cmake\android.toolchain.cmake -DANDROID_PLATFORM=%platformstr% -DANDROID_ABI=%%A^
 %CMAKE_DEFINITIONS%
cmake --build .
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
xcopy ..\include ..\..\include /i /s /y
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
copy .\lib\libvkzos.a ..\..\lib\%%A
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
cd ..\..\
rmdir /Q /S vkzos-0.0.1
del vkzos-0.0.1.tar

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

7z x ogg-1.3.3.tar.gz
if "!errorlevel!" neq "0" endlocal & exit /b !errorlevel! 
7z x ogg-1.3.3.tar
cd ogg-1.3.3
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
rmdir /Q /S ogg-1.3.3
del ogg-1.3.3.tar
del pax_global_header

7z x vorbis-1.3.6.tar.gz
if "!errorlevel!" neq "0" endlocal & exit /b !errorlevel! 
7z x vorbis-1.3.6.tar
cd vorbis-1.3.6
mkdir build
cd build
cmake .. -G"MinGW Makefiles" -DBUILD_SHARED_LIBS=OFF^
 -DOGG_INCLUDE_DIRS=%depspath%/include -DOGG_LIBRARIES=%depspath%/lib/%%A/libogg.a^
 -DCMAKE_TOOLCHAIN_FILE=%NDK%\build\cmake\android.toolchain.cmake -DANDROID_PLATFORM=%platformstr% -DANDROID_ABI=%%A^
 %CMAKE_DEFINITIONS%
cmake --build .
if "!errorlevel!" neq "0" endlocal & exit /b !errorlevel! 
xcopy ..\include\vorbis ..\..\include\vorbis /i /s /y
if "!errorlevel!" neq "0" endlocal & exit /b !errorlevel! 
copy lib\*.a ..\..\lib\%%A
if "!errorlevel!" neq "0" endlocal & exit /b !errorlevel! 
cd ..\..\
rmdir /Q /S vorbis-1.3.6
del vorbis-1.3.6.tar
del pax_global_header

7z x freetype-2.11.0.tar.gz
if "!errorlevel!" neq "0" endlocal & exit /b !errorlevel! 
7z x freetype-2.11.0.tar
cd freetype-2.11.0
mkdir build
cd build
cmake .. -G"MinGW Makefiles" -DBUILD_SHARED_LIBS=OFF^
 -DCMAKE_TOOLCHAIN_FILE=%NDK%\build\cmake\android.toolchain.cmake -DANDROID_PLATFORM=%platformstr% -DANDROID_ABI=%%A^
 %CMAKE_DEFINITIONS%
cmake --build .
if "!errorlevel!" neq "0" endlocal & exit /b !errorlevel! 
xcopy ..\include ..\..\include /s /e /y
if "!errorlevel!" neq "0" endlocal & exit /b !errorlevel!
set FREETYPEBIN=libfreetype.a
if /I "%~1" == "Debug" set FREETYPEBIN=libfreetyped.a
copy !FREETYPEBIN! ..\..\lib\%%A\libfreetype.a
if "!errorlevel!" neq "0" endlocal & exit /b !errorlevel! 
cd ..\..
rmdir /Q /S freetype-2.11.0
del freetype-2.11.0.tar
)

@echo small3d dependencies built successfully for Android (%~1 mode)

@endlocal
