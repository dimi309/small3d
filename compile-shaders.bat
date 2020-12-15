@echo off
setlocal enabledelayedexpansion
cd resources\shaders
if "%errorlevel%" neq "0" endlocal & exit /b %errorlevel%
glslangvalidator -V perspectiveMatrixLightedShader.vert -o perspectiveMatrixLightedShader.spv
if "%errorlevel%" neq "0" endlocal & exit /b %errorlevel%
glslangvalidator -V textureShader.frag -o textureShader.spv
if "%errorlevel%" neq "0" endlocal & exit /b %errorlevel%
echo SPV binaries created successfully
if exist ..\..\build\shaders\perspectiveMatrixLightedShader.spv (
for /r %%a in (*.spv) do (
echo Copying %%a to build\shaders
copy /y "%%a" ..\..\build\shaders
if "%errorlevel%" neq "0" endlocal & exit /b %errorlevel%
)
for /r %%a in (*.spv) do (
echo Copying %%a to build\bin\resources\shaders
copy /y "%%a" ..\..\build\bin\resources\shaders
if "%errorlevel%" neq "0" endlocal & exit /b %errorlevel%
)
)
cd ..\..
