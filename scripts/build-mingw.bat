@echo off
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

cd ..\deps\scripts
if exist include rmdir /Q /S include
if exist lib rmdir /Q /S lib
call prepare-mingw %1 %2
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
cd ..
if exist build rmdir /Q /S build

mkdir build
cd build

cmake .. -G"MinGW Makefiles" %CMAKE_DEFINITIONS%
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
cmake --build .
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%

cd ..\scripts

if "%errorlevel%" neq "0" endlocal & exit /b %errorlevel%

echo small3d built successfully for MinGW (%~1 mode)
