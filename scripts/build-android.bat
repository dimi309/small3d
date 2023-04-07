@echo off

REM For this to work, set the %NDK% variable to your ndk path. It should look like
REM C:\Users\user\AppData\Local\Android\Sdk\ndk\22.1.7171670 for example. Also, the
REM script needs to run in an environment with MinGW set up and no settings for
REM Visual Studio.
REM Because of some OpenGL ES details, an NDK version that works well is 22.1.7171670. There
REM can be some glitches on newer versions.

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

set sourcepath=%cd%

set platformstr=android-16

echo Building on %platformstr%

if /I not "%~2" == "skipdeps"  (
cd ..\deps\scripts
if exist include rmdir /Q /S include
if exist lib rmdir /Q /S lib
call prepare-android.bat %1 %platformstr%
if %errorlevel% neq 0 endlocal & exit /b %errorlevel%
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

cd ..\resources\shadersOpenGLES
mkdir ..\..\build\shaders\
for /r %%a in (*.*) do (
echo Copying %%a to build\shaders
copy /y "%%a" ..\..\build\shaders\
if "%errorlevel%" neq "0" endlocal & exit /b %errorlevel%
)
cd ..\..

if exist android\app\src\main\assets\resources (
mkdir android\app\src\main\assets\resources\shaders
cd resources\shadersOpenGLES
for /r %%a in (*.*) do (
echo Copying %%a to android\app\src\main\assets\resources\shaders
copy /y "%%a" ..\..\android\app\src\main\assets\resources\shaders\
if "%errorlevel%" neq "0" endlocal & exit /b %errorlevel%
)
)
cd ..\..

echo Copying small3d_android.h to build\include
copy /y src\small3d_android.h build\include\

echo small3d built successfully for Android (%~1 mode)
endlocal
@echo on
