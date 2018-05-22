set VSCONFIG=Visual Studio 15 2017 Win64
set BUILDTYPE=Release

mkdir include
mkdir lib

7z x glfw-master-20180409.zip
if %errorlevel% neq 0 exit /b %errorlevel%
cd glfw-master
mkdir build
cd build
cmake .. -G"%VSCONFIG%" -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF -DGLFW_INSTALL=OFF
cmake --build . --config %BUILDTYPE%
if %errorlevel% neq 0 exit /b %errorlevel%
xcopy ..\include\GLFW ..\..\include\GLFW /i /s
copy src\%BUILDTYPE%\glfw3.lib ..\..\lib\
cd ..\..
rmdir /Q /S glfw-master

7z x glew-2.1.0.zip
if %errorlevel% neq 0 exit /b %errorlevel%
cd glew-2.1.0
cmake -G"%VSCONFIG%" build/cmake -DBUILD_UTILS=OFF
cmake --build . --config %BUILDTYPE%
if %errorlevel% neq 0 exit /b %errorlevel%
xcopy include\GL ..\include\GL /i /s
copy lib\%BUILDTYPE%\libglew32.lib ..\lib\glew.lib
cd ..
rmdir /Q /S glew-2.1.0

7z x glm-master-20180508.zip
if %errorlevel% neq 0 exit /b %errorlevel%
xcopy glm-master\glm include\glm /i /s
rmdir /Q /S glm-master

7z x zlib-1.2.11.tar.gz
if %errorlevel% neq 0 exit /b %errorlevel%
7z x zlib-1.2.11.tar
cd zlib-1.2.11
mkdir build
cd build
cmake .. -G"%VSCONFIG%"
cmake --build . --config %BUILDTYPE%
if %errorlevel% neq 0 exit /b %errorlevel%
copy ..\zlib.h ..\..\include
copy zconf.h ..\..\include
copy %BUILDTYPE%\zlibstatic.lib ..\..\lib\zlib.lib
cd ..\..\
rmdir /Q /S zlib-1.2.11
del zlib-1.2.11.tar

7z x libpng-1.6.34.tar.gz
if %errorlevel% neq 0 exit /b %errorlevel%
7z x libpng-1.6.34.tar
cd libpng-1.6.34
mkdir build
cd build
cmake .. -G"%VSCONFIG%" -DPNG_SHARED=OFF -DPNG_STATIC=ON -DPNG_TESTS=OFF -DZLIB_LIBRARY=..\..\lib/zlib.lib -DZLIB_INCLUDE_DIR=..\..\include
cmake --build . --config %BUILDTYPE%
if %errorlevel% neq 0 exit /b %errorlevel%
copy ..\*.h ..\..\include
copy pnglibconf.h ..\..\include
copy %BUILDTYPE%\libpng16_static.lib ..\..\lib\png.lib
cd ..\..\
rmdir /Q /S libpng-1.6.34
del libpng-1.6.34.tar
del pax_global_header

REM Using slightly modified (not-strict) version because the regular one makes VS 2017 crash.
REM This one has a slightly modified CMakeLists.txt file. Look for "small3d" in it, to see
REM what is different.
7z x googletest-release-1.8.0-not-strict.tar.gz
if %errorlevel% neq 0 exit /b %errorlevel%
7z x googletest-release-1.8.0-not-strict.tar
cd googletest-release-1.8.0-not-strict
mkdir build
cd build
cmake .. -G"%VSCONFIG%" -DBUILD_GMOCK=OFF -DBUILD_GTEST=ON -Dgtest_disable_pthreads=ON
cmake --build . --config %BUILDTYPE%
if %errorlevel% neq 0 exit /b %errorlevel%
xcopy ..\googletest\include\gtest ..\..\include\gtest /i /s
copy googletest\%BUILDTYPE%\gtest.lib ..\..\lib
copy googletest\%BUILDTYPE%\gtest_main.lib ..\..\lib\
cd ..\..
rmdir /Q /S googletest-release-1.8.0-not-strict
del googletest-release-1.8.0-not-strict.tar
del pax_global_header

7z x ogg-1.3.3.tar.gz
if %errorlevel% neq 0 exit /b %errorlevel%
7z x ogg-1.3.3.tar
cd ogg-1.3.3
mkdir build
cd build
cmake .. -G"%VSCONFIG%" -DBUILD_SHARED_LIBS=OFF
cmake --build . --config %BUILDTYPE%
if %errorlevel% neq 0 exit /b %errorlevel%
xcopy ..\include\ogg ..\..\include\ogg /i /s
copy include\ogg\config_types.h ..\..\include\ogg
copy %BUILDTYPE%\ogg.lib ..\..\lib
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
cmake .. -G"%VSCONFIG%" -DBUILD_SHARED_LIBS=OFF -DOGG_ROOT=%depspath%
cmake --build . --config %BUILDTYPE%
if %errorlevel% neq 0 exit /b %errorlevel%
xcopy ..\include\vorbis ..\..\include\vorbis /i /s
copy lib\%BUILDTYPE%\*.lib ..\..\lib
cd ..\..\
rmdir /Q /S vorbis-1.3.6
del vorbis-1.3.6.tar
del pax_global_header

7z x pa_stable_v190600_20161030.tgz
if %errorlevel% neq 0 exit /b %errorlevel%
7z x pa_stable_v190600_20161030.tar
cd portaudio
mkdir build1
cd build1
cmake .. -G"%VSCONFIG%" -DPA_USE_WDMKS=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build . --config %BUILDTYPE%
if %errorlevel% neq 0 exit /b %errorlevel%
copy ..\include\* ..\..\include
copy %BUILDTYPE%\portaudio_static*.lib ..\..\lib\portaudio_static.lib
cd ..\..\
rmdir /Q /S portaudio
del pa_stable_v190600_20161030.tar

7z x freetype-2.9.tar.gz
if %errorlevel% neq 0 exit /b %errorlevel%
7z x freetype-2.9.tar
cd freetype-2.9
mkdir build
cd build
cmake .. -G"%VSCONFIG%" -DBUILD_SHARED_LIBS=OFF
cmake --build . --config %BUILDTYPE%
if %errorlevel% neq 0 exit /b %errorlevel%
xcopy ..\include ..\..\include /s /e
copy %BUILDTYPE%\freetype*.lib ..\..\lib
cd ..\..
rmdir /Q /S freetype-2.9
del freetype-2.9.tar

