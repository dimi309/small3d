REM For this to work, set the %NDK% variable to your ndk path. It should look like
REM C:\Users\user\AppData\Local\Android\Sdk\ndk\21.2.6472646 for example. Also, the
REM script needs to run in an environment with MinGW set up and no settings for
REM Visual Studio.

mkdir include
mkdir lib

SET depspath=%cd%
SET platformstr=android-28

7z x glm-0.9.9.0.zip
if errorlevel 1 exit /B 
xcopy glm\glm include\glm /i /s /y
rmdir /Q /S glm

for %%A in (x86,x86_64,armeabi-v7a,arm64-v8a) do (

mkdir lib\%%A

7z x libpng-1.6.37.tar.gz
if errorlevel 1 exit /B
7z x libpng-1.6.37.tar
cd libpng-1.6.37
mkdir build
cd build
cmake .. -G"MinGW Makefiles" -DPNG_SHARED=OFF -DPNG_STATIC=ON -DPNG_TESTS=OFF^
 -DCMAKE_TOOLCHAIN_FILE=%NDK%\build\cmake\android.toolchain.cmake -DANDROID_PLATFORM=%platformstr% -DANDROID_ABI=%%A
cmake --build .
if errorlevel 1 exit /B
copy ..\*.h ..\..\include /y
copy pnglibconf.h ..\..\include /y
copy libpng.a ..\..\lib\%%A
cd ..\..\
rmdir /Q /S libpng-1.6.37
del libpng-1.6.37.tar
del pax_global_header

7z x ogg-1.3.3.tar.gz
if errorlevel 1 exit /B
7z x ogg-1.3.3.tar
cd ogg-1.3.3
mkdir build
cd build
cmake .. -G"MinGW Makefiles" -DBUILD_SHARED_LIBS=OFF^
 -DCMAKE_TOOLCHAIN_FILE=%NDK%\build\cmake\android.toolchain.cmake -DANDROID_PLATFORM=%platformstr% -DANDROID_ABI=%%A
cmake --build .
if errorlevel 1 exit /B
xcopy ..\include\ogg ..\..\include\ogg /i /s /y
copy include\ogg\config_types.h ..\..\include\ogg /y
copy libogg.a ..\..\lib\%%A
cd ..\..\
rmdir /Q /S ogg-1.3.3
del ogg-1.3.3.tar
del pax_global_header

7z x vorbis-1.3.6.tar.gz
if errorlevel 1 exit /B
7z x vorbis-1.3.6.tar
cd vorbis-1.3.6
mkdir build
cd build
cmake .. -G"MinGW Makefiles" -DBUILD_SHARED_LIBS=OFF^
 -DOGG_INCLUDE_DIRS=%depspath%/include -DOGG_LIBRARIES=%depspath%/lib/%%A/libogg.a^
 -DCMAKE_TOOLCHAIN_FILE=%NDK%\build\cmake\android.toolchain.cmake -DANDROID_PLATFORM=%platformstr% -DANDROID_ABI=%%A
cmake --build .
if errorlevel 1 exit /B
xcopy ..\include\vorbis ..\..\include\vorbis /i /s /y
copy lib\*.a ..\..\lib\%%A
cd ..\..\
rmdir /Q /S vorbis-1.3.6
del vorbis-1.3.6.tar
del pax_global_header

7z x freetype-2.9.1.tar.gz
if errorlevel 1 exit /B
7z x freetype-2.9.1.tar
cd freetype-2.9.1
mkdir build
cd build
cmake .. -G"MinGW Makefiles" -DBUILD_SHARED_LIBS=OFF^
 -DCMAKE_TOOLCHAIN_FILE=%NDK%\build\cmake\android.toolchain.cmake -DANDROID_PLATFORM=%platformstr% -DANDROID_ABI=%%A
cmake --build .
if errorlevel 1 exit /B
xcopy ..\include ..\..\include /s /e /y
copy libfreetype.a ..\..\lib\%%A
cd ..\..
rmdir /Q /S freetype-2.9.1
del freetype-2.9.1.tar

)
