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
glslangvalidator -V perspectiveMatrixLightedShader.vert -o perspectiveMatrixLightedShader.spv %debug_info%
if "%errorlevel%" neq "0" endlocal & exit /b %errorlevel%
glslangvalidator -V perspectiveMatrixLightedShaderNoJoints.vert -o perspectiveMatrixLightedShaderNoJoints.spv %debug_info%
if "%errorlevel%" neq "0" endlocal & exit /b %errorlevel%
glslangvalidator -V textureShader.frag -o textureShader.spv %debug_info%
if "%errorlevel%" neq "0" endlocal & exit /b %errorlevel%
echo SPV binaries created successfully
if exist ..\..\build (
if not exist ..\..\build\shaders mkdir ..\..\build\shaders
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

