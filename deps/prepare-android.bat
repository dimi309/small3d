REM For this to work, set the %NDK% variable to your ndk path. It should look like
REM C:\Users\me\AppData\Local\Android\Sdk\ndk\20.0.5594570 for example. Also, the
REM script needs to run in an environment with MinGW set up and no settings for
REM Visual Studio.

mkdir include
mkdir lib

7z x glm-0.9.9.0.zip
if %errorlevel% neq 0 exit /b %errorlevel% 
xcopy glm\glm include\glm /i /s /y
rmdir /Q /S glm

for %%A in (x86,x86_64,armeabi-v7a,arm64-v8a) do (

set CMAKE_DEFINITIONS=-DCMAKE_TOOLCHAIN_FILE=%NDK%\build\cmake\android.toolchain.cmake -DANDROID_PLATFORM=android-28 -DANDROID_ABI=%%A

mkdir lib\%%A

7z x libpng-1.6.34.tar.gz
if %errorlevel% neq 0 exit /b %errorlevel%
7z x libpng-1.6.34.tar
cd libpng-1.6.34
mkdir build
cd build
cmake .. -G"MinGW Makefiles" -DPNG_SHARED=OFF -DPNG_STATIC=ON -DPNG_TESTS=OFF  %CMAKE_DEFINITIONS%
cmake --build .
if %errorlevel% neq 0 exit /b %errorlevel%
copy ..\*.h ..\..\include /y
copy pnglibconf.h ..\..\include /y
copy libpng.a ..\..\lib\%%A
cd ..\..\
rmdir /Q /S libpng-1.6.34
del libpng-1.6.34.tar
del pax_global_header

7z x ogg-1.3.3.tar.gz
if %errorlevel% neq 0 exit /b %errorlevel%
7z x ogg-1.3.3.tar
cd ogg-1.3.3
mkdir build
cd build
cmake .. -G"MinGW Makefiles" -DBUILD_SHARED_LIBS=OFF %CMAKE_DEFINITIONS%
cmake --build .
if %errorlevel% neq 0 exit /b %errorlevel%
xcopy ..\include\ogg ..\..\include\ogg /i /s /y
copy include\ogg\config_types.h ..\..\include\ogg /y
copy libogg.a ..\..\lib\%%A
cd ..\..\
rmdir /Q /S ogg-1.3.3
del ogg-1.3.3.tar
del pax_global_header

set depspath=%cd%

7z x vorbis-1.3.6.tar.gz
if %errorlevel% neq 0 exit /b %errorlevel%
7z x vorbis-1.3.6.tar
cd vorbis-1.3.6
mkdir build
cd build
cmake .. -G"MinGW Makefiles" -DBUILD_SHARED_LIBS=OFF -DOGG_INCLUDE_DIRS=%depspath%/include -DOGG_LIBRARIES=%depspath%/lib/%%A/libogg.a %CMAKE_DEFINITIONS%
cmake --build .
if %errorlevel% neq 0 exit /b %errorlevel%
xcopy ..\include\vorbis ..\..\include\vorbis /i /s /y
copy lib\*.a ..\..\lib\%%A
cd ..\..\
rmdir /Q /S vorbis-1.3.6
del vorbis-1.3.6.tar
del pax_global_header

7z x freetype-2.9.1.tar.gz
if %errorlevel% neq 0 exit /b %errorlevel%
7z x freetype-2.9.1.tar
cd freetype-2.9.1
mkdir build
cd build
cmake .. -G"MinGW Makefiles" -DBUILD_SHARED_LIBS=OFF %CMAKE_DEFINITIONS%
cmake --build .
if %errorlevel% neq 0 exit /b %errorlevel%
xcopy ..\include ..\..\include /s /e /y
copy libfreetype.a ..\..\lib\%%A
cd ..\..
rmdir /Q /S freetype-2.9.1
del freetype-2.9.1.tar

)


