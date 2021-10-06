@echo off
setlocal enabledelayedexpansion

set args_ok=false

if /I "%~1" == "debug" (
set args_ok=true
set debug_info=-g
)
if /I "%~1" == "release" (
set args_ok=true
set debug_info=-g0
)

if "%args_ok%" == "false" (
echo Please indicate build type: debug or release
endlocal & exit /b 1
)

cd ..\resources\shaders
if "%errorlevel%" neq "0" endlocal & exit /b %errorlevel%
glslangvalidator -V perspectiveMatrixLightedShader.vert -o perspectiveMatrixLightedShader.spv -g0
if "%errorlevel%" neq "0" endlocal & exit /b %errorlevel%
glslangvalidator -V textureShader.frag -o textureShader.spv -g0
if "%errorlevel%" neq "0" endlocal & exit /b %errorlevel%
echo SPV binaries created successfully
if exist ..\..\build\shaders\perspectiveMatrixLightedShader.spv (
for /r %%a in (*.spv) do (
echo Copying %%a to build\shaders
copy /y "%%a" ..\..\build\shaders
if "%errorlevel%" neq "0" endlocal & exit /b %errorlevel%
)
if exist ..\..\build\bin\resources\shaders (
for /r %%a in (*.spv) do (
echo Copying %%a to build\bin\resources\shaders
copy /y "%%a" ..\..\build\bin\resources\shaders
if "%errorlevel%" neq "0" endlocal & exit /b %errorlevel%
)
)
)

