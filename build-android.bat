@echo off

REM For this to work, set the %NDK% variable to your ndk path. It should look like
REM C:\Users\user\AppData\Local\Android\Sdk\ndk\22.0.6917172 for example. Also, the
REM script needs to run in an environment with MinGW set up and no settings for
REM Visual Studio.

setlocal enabledelayedexpansion

set args_ok=false

if /I "%~1" == "Debug" set args_ok=true
if /I "%~1" == "Release" set args_ok=true

if "%args_ok%" == "false" (
echo Please indicate build type: Debug or Release
endlocal & exit /B 1
)

if /I "%~1" == "Debug" set CMAKE_DEFINITIONS=-DCMAKE_BUILD_TYPE=Debug
if /I "%~1" == "Release" set CMAKE_DEFINITIONS=

@echo on

set sourcepath=%cd%
set platformstr=android-26

if exist build (
echo Build directory exists!
endlocal & exit /B
)

echo Build directory does not exist (good)...

mkdir build
cd build

for %%A in (x86,x86_64,armeabi-v7a,arm64-v8a) do (
cmake .. -G"MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE=%NDK%\build\cmake\android.toolchain.cmake ^
-DANDROID_PLATFORM=%platformstr% -DANDROID_ABI=%%A %CMAKE_DEFINITIONS%
cmake --build .
if "!errorlevel!" neq "0" endlocal & exit /b %errorlevel% 
move lib\*.a lib\%%A
if "!errorlevel!" neq "0" endlocal & exit /b %errorlevel% 
del /Q *.*
rmdir /Q /S CMakeFiles
rmdir /Q /S src
)

@echo off
if /I "%~1" == "Release" (
echo Warning: Did not set cmake build type to release explicitly because that leads to the following Vulkan related error on some devices: I/Adreno: Shader compilation failed for shaderType: 0
)
echo small3d built successfully for Android (%~1 mode)
endlocal
@echo on
