@echo off
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

set VSCONFIG=-G"Visual Studio 17 2022" -A x64 -DCMAKE_CXX_STANDARD=17

cd ..\deps\scripts
if exist include rmdir /Q /S include
if exist lib rmdir /Q /S lib
call prepare-vs.bat %1 %2
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
cd ..
if exist build rmdir /Q /S build

mkdir build
cd build

cmake .. %VSCONFIG%
if "%errorlevel%" neq "0" endlocal & exit /b %errorlevel%
cmake --build . --config %BUILDTYPE%
if "%errorlevel%" neq "0" endlocal & exit /b %errorlevel%

cd ..\scripts

if "%errorlevel%" neq "0" endlocal & exit /b %errorlevel%

echo small3d built successfully for Visual Studio (%~1 mode)
