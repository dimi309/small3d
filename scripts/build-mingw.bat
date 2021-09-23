@echo off
setlocal enabledelayedexpansion
set args_ok=false
set opengl_ok=false

if /I "%~1" == "debug" set args_ok=true
if /I "%~1" == "release" set args_ok=true
if /I "%~2" == "" set opengl_ok=true
if /I "%~2" == "opengl" set opengl_ok=true
if not "%opengl_ok%" == "true" set args_ok=false

if "%args_ok%" == "false" (
echo Please indicate build type: debug or release, followed by opengl if you would like an opengl build.
endlocal & exit /b 1
)

if /I "%~1" == "debug" set CMAKE_DEFINITIONS=-DCMAKE_BUILD_TYPE=Debug
if /I "%~1" == "release" set CMAKE_DEFINITIONS=-DCMAKE_BUILD_TYPE=Release

if /I "%~2" == "opengl" set CMAKE_DEFINITIONS=%CMAKE_DEFINITIONS% -DSMALL3D_OPENGL=ON

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
echo small3d built successfully for MinGW (%~1 mode)
