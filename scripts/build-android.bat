@echo off

REM For this to work, set the %NDK% variable to your ndk path. It should look like
REM C:\Users\user\AppData\Local\Android\Sdk\ndk\22.0.6917172 for example. Also, the
REM script needs to run in an environment with MinGW set up and no settings for
REM Visual Studio.
REM For OpenGL ES builds, an NDK version that works well is 22.1.7171670. There
REM can be some glitches on newer versions.

setlocal enabledelayedexpansion

set args_ok=false

if /I "%~1" == "Debug" set args_ok=true
if /I "%~1" == "Release" set args_ok=true
if /I "%~2" == "" set opengles_ok=true
if /I "%~2" == "opengles" set opengles_ok=true
if /I "%~2" == "skipdeps" set opengles_ok=true
if not "%opengles_ok%" == "true" set args_ok=false

if "%args_ok%" == "false" (
echo Please indicate build type: Debug or Release, followed by opengles if you would like to build in OpenGL ES based mode.
endlocal & exit /b 1
)

if /I "%~1" == "Debug" set CMAKE_DEFINITIONS=-DCMAKE_BUILD_TYPE=Debug
if /I "%~1" == "Release" set CMAKE_DEFINITIONS=-DCMAKE_BUILD_TYPE=Release
if /I "%~2" == "opengles" set CMAKE_DEFINITIONS=%CMAKE_DEFINITIONS% -DSMALL3D_OPENGL=ON

set sourcepath=%cd%

if /I "%~2" == "opengles" (
set platformstr=android-16
) else (
set platformstr=android-26
)

echo Building on %platformstr%

if /I not "%~2" == "skipdeps"  (
if /I not "%~3" == "skipdeps"  (
cd ..\deps\scripts
if exist include rmdir /Q /S include
if exist lib rmdir /Q /S lib
if /I "%~2" == "opengles" (
call prepare-android.bat %1 %platformstr% opengles
) else (
call prepare-android.bat %1 %platformstr%
)
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
)
)
cd ..
if exist build rmdir /Q /S build

mkdir build
cd build

for %%A in (x86,x86_64,armeabi-v7a,arm64-v8a) do (
cmake .. -G"MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE=%NDK%\build\cmake\android.toolchain.cmake ^
-DANDROID_PLATFORM=%platformstr% -DANDROID_ABI=%%A %CMAKE_DEFINITIONS%
cmake --build .
if "!errorlevel!" neq "0" endlocal & exit /b !errorlevel! 
move lib\*.a lib\%%A
if "!errorlevel!" neq "0" endlocal & exit /b !errorlevel! 
del /Q *.*
rmdir /Q /S CMakeFiles
rmdir /Q /S src
)

cd ..\scripts

if /I not "%~2" == "opengles" (
call compile-shaders.bat %~1
echo Copying android\app\CMakeListsVulkan.txt to android\app\CMakeLists.txt
copy /y ..\android\app\CMakeListsVulkan.txt ..\android\app\CMakeLists.txt
if "%errorlevel%" neq "0" endlocal & exit /b %errorlevel%
)
cd ..

if /I "%~2" == "opengles" (

cd opengl\resources\shadersOpenGLES
mkdir ..\..\..\build\shaders\
for /r %%a in (*.*) do (
echo Copying %%a to build\shaders
copy /y "%%a" ..\..\..\build\shaders\
if "%errorlevel%" neq "0" endlocal & exit /b %errorlevel%
)
cd ..\..\..

if exist android\app\src\main\assets\resources (
mkdir android\app\src\main\assets\resources\shaders
cd opengl\resources\shadersOpenGLES
for /r %%a in (*.*) do (
echo Copying %%a to android\app\src\main\assets\resources\shaders
copy /y "%%a" ..\..\..\android\app\src\main\assets\resources\shaders\
if "%errorlevel%" neq "0" endlocal & exit /b %errorlevel%
)
)
cd ..\..\..
echo Copying android\app\CMakeListsOpenGLES.txt to android\app\CMakeLists.txt
copy /y android\app\CMakeListsOpenGLES.txt android\app\CMakeLists.txt
if "%errorlevel%" neq "0" endlocal & exit /b %errorlevel%
)

echo Copying small3d_android.h to build\include
copy /y src\small3d_android.h build\include\

echo small3d built successfully for Android (%~1 mode)
endlocal
@echo on
