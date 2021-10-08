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
echo Please indicate build type: debug or release, followed by opengl if you would like to also prepare OpenGL-related libraries.
endlocal & exit /b 1
)

if /I "%~1" == "debug" set BUILDTYPE=Debug
if /I "%~1" == "release" set BUILDTYPE=Release

set CMAKE_DEFINITIONS=
if /I "%~2" == "opengl" set CMAKE_DEFINITIONS=%CMAKE_DEFINITIONS% -DSMALL3D_OPENGL=ON

set VSCONFIG=-G"Visual Studio 16 2019" -A x64

cd ..\deps\scripts
if exist include rmdir /Q /S include
if exist lib rmdir /Q /S lib
call prepare-vs.bat %1 %2
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
cd ..
if exist build rmdir /Q /S build

mkdir build
cd build

cmake .. %VSCONFIG% %CMAKE_DEFINITIONS%
if "%errorlevel%" neq "0" endlocal & exit /b %errorlevel%
cmake --build . --config %BUILDTYPE%
if "%errorlevel%" neq "0" endlocal & exit /b %errorlevel%

cd ..\scripts

if /I "%~2" neq "opengl" (
compile-shaders.bat %~1
)
if "%errorlevel%" neq "0" endlocal & exit /b %errorlevel%

echo small3d built successfully for Visual Studio (%~1 mode)
