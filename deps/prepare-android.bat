REM ABIs:
REM x86
REM x86_64
REM armeabi-v7a
REM arm64-v8a

set CMAKE_DEFINITIONS=-DCMAKE_TOOLCHAIN_FILE=%NDK%\build\cmake\android.toolchain.cmake -DANDROID_PLATFORM=android-28 -DANDROID_ABI=armeabi-v7a

mkdir include
mkdir lib

7z x glm-0.9.9.0.zip
if %errorlevel% neq 0 exit /b %errorlevel% 
xcopy glm\glm include\glm /i /s
rmdir /Q /S glm

7z x libpng-1.6.34.tar.gz
if %errorlevel% neq 0 exit /b %errorlevel%
7z x libpng-1.6.34.tar
cd libpng-1.6.34
mkdir build
cd build
cmake .. -G"MinGW Makefiles" -DPNG_SHARED=OFF -DPNG_STATIC=ON -DPNG_TESTS=OFF  %CMAKE_DEFINITIONS%
cmake --build .
if %errorlevel% neq 0 exit /b %errorlevel%
copy ..\*.h ..\..\include
copy pnglibconf.h ..\..\include
copy libpng.a ..\..\lib
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
xcopy ..\include\ogg ..\..\include\ogg /i /s
copy include\ogg\config_types.h ..\..\include\ogg
copy libogg.a ..\..\lib
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
cmake .. -G"MinGW Makefiles" -DBUILD_SHARED_LIBS=OFF -DOGG_INCLUDE_DIRS=%depspath%/include -DOGG_LIBRARIES=%depspath%/lib/libogg.a %CMAKE_DEFINITIONS%
cmake --build .
if %errorlevel% neq 0 exit /b %errorlevel%
xcopy ..\include\vorbis ..\..\include\vorbis /i /s
copy lib\*.a ..\..\lib
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
xcopy ..\include ..\..\include /s /e
copy libfreetype.a ..\..\lib
cd ..\..
rmdir /Q /S freetype-2.9.1
del freetype-2.9.1.tar
