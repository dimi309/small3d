REM For this to work, set the %NDK% variable to your ndk path. It should look like
REM C:\Users\user\AppData\Local\Android\Sdk\ndk\22.0.6917172 for example. Also, the
REM script needs to run in an environment with MinGW set up and no settings for
REM Visual Studio.

SET sourcepath=%cd%
SET platformstr=android-28


if exist build (
echo Build directory exists!
exit /B
)

echo Build directory does not exist (good)...

mkdir build
cd build

for %%A in (x86,x86_64,armeabi-v7a,arm64-v8a) do (

cmake .. -G"MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE=%NDK%\build\cmake\android.toolchain.cmake ^
-DANDROID_PLATFORM=%platformstr% -DANDROID_ABI=%%A
cmake --build .
if errorlevel 1 exit /B
move lib\*.a lib\%%A
del /Q *.*
rmdir /Q /S CMakeFiles
rmdir /Q /S src

)







