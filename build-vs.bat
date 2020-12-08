@echo off
setlocal enabledelayedexpansion
set args_ok=false

if /I "%~1" == "Debug" set args_ok=true
if /I "%~1" == "Release" set args_ok=true

if "%args_ok%" == "false" (
echo Please indicate build type: Debug or Release
endlocal & exit /b 1
)

if /I "%~1" == "Debug" set BUILDTYPE=Debug
if /I "%~1" == "Release" set BUILDTYPE=Release

set VSCONFIG=-G"Visual Studio 16 2019" -A x64

if exist build (
echo Build directory exists!
endlocal & exit /B
)

echo Build directory does not exist (good)...

mkdir build
cd build

cmake .. %VSCONFIG%
if "%errorlevel%" neq "0" endlocal & exit /b %errorlevel%
cmake --build . --config %BUILDTYPE%
if "%errorlevel%" neq "0" endlocal & exit /b %errorlevel%
echo small3d built successfully for Visual Studio (%~1 mode)
