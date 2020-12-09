@echo off
setlocal enabledelayedexpansion
set args_ok=false

if /I "%~1" == "Debug" set args_ok=true
if /I "%~1" == "Release" set args_ok=true

if "%args_ok%" == "false" (
echo Please indicate build type: Debug or Release
endlocal & exit /b 1
)

if /I "%~1" == "Debug" set CMAKE_DEFINITIONS=-DCMAKE_BUILD_TYPE=Debug
if /I "%~1" == "Release" set CMAKE_DEFINITIONS=-DCMAKE_BUILD_TYPE=Release

if exist build (
echo Build directory exists!
endlocal & exit /B
)

echo Build directory does not exist (good)...

mkdir build
cd build

cmake .. -G"MinGW Makefiles" %CMAKE_DEFINITIONS%
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
cmake --build .
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
echo small3d built successfully for MinGW (%~1 mode)
