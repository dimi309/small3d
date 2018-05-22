mkdir include
mkdir lib
7z x glfw-master-20180409.zip
if %errorlevel% neq 0 exit /b %errorlevel%
cd glfw-master
mkdir build
cd build
cmake .. -G"MinGW Makefiles" -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF -DGLFW_INSTALL=OFF
cmake --build .
if %errorlevel% neq 0 exit /b %errorlevel%
xcopy ..\include\GLFW ..\..\include\GLFW /i /s
copy src\libglfw3.a ..\..\lib\
cd ..\..
rmdir /Q /S glfw-master

7z x glew-2.1.0.zip
if %errorlevel% neq 0 exit /b %errorlevel%
cd glew-2.1.0
cmake -G"MinGW Makefiles" build/cmake -DBUILD_UTILS=OFF
cmake --build .
if %errorlevel% neq 0 exit /b %errorlevel%
xcopy include\GL ..\include\GL /i /s
copy lib\libglew32.a ..\lib\
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
cmake .. -G"MinGW Makefiles"
cmake --build .
if %errorlevel% neq 0 exit /b %errorlevel%
copy ..\zlib.h ..\..\include
copy zconf.h ..\..\include
copy libzlibstatic.a ..\..\lib\zlib.a
cd ..\..\
rmdir /Q /S zlib-1.2.11
del zlib-1.2.11.tar

7z x libpng-1.6.34.tar.gz
if %errorlevel% neq 0 exit /b %errorlevel%
7z x libpng-1.6.34.tar
cd libpng-1.6.34
mkdir build
cd build
cmake .. -G"MinGW Makefiles" -DPNG_SHARED=OFF -DPNG_STATIC=ON -DPNG_TESTS=OFF -DZLIB_LIBRARY=..\..\lib/zlib.a -DZLIB_INCLUDE_DIR=..\..\include
cmake --build .
if %errorlevel% neq 0 exit /b %errorlevel%
copy ..\*.h ..\..\include
copy pnglibconf.h ..\..\include
copy libpng.a ..\..\lib
cd ..\..\
rmdir /Q /S libpng-1.6.34
del libpng-1.6.34.tar
del pax_global_header

7z x googletest-release-1.8.0.tar.gz
if %errorlevel% neq 0 exit /b %errorlevel%
7z x googletest-release-1.8.0.tar
cd googletest-release-1.8.0
mkdir build
cd build
cmake .. -G"MinGW Makefiles" -DBUILD_GMOCK=OFF -DBUILD_GTEST=ON -Dgtest_disable_pthreads=ON
cmake --build .
if %errorlevel% neq 0 exit /b %errorlevel%
xcopy ..\googletest\include\gtest ..\..\include\gtest /i /s
copy googletest\libgtest.a ..\..\lib
copy googletest\libgtest_main.a ..\..\lib\
cd ..\..
rmdir /Q /S googletest-release-1.8.0
del googletest-release-1.8.0.tar
del pax_global_header

7z x ogg-1.3.3.tar.gz
if %errorlevel% neq 0 exit /b %errorlevel%
7z x ogg-1.3.3.tar
cd ogg-1.3.3
mkdir build
cd build
cmake .. -G"MinGW Makefiles" -DBUILD_SHARED_LIBS=OFF
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
cmake .. -G"MinGW Makefiles" -DBUILD_SHARED_LIBS=OFF -DOGG_ROOT=%depspath%
cmake --build .
if %errorlevel% neq 0 exit /b %errorlevel%
xcopy ..\include\vorbis ..\..\include\vorbis /i /s
copy lib\*.a ..\..\lib
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
cmake .. -G"MinGW Makefiles" -DPA_USE_WDMKS=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build .
if %errorlevel% neq 0 exit /b %errorlevel%
copy ..\include\* ..\..\include
copy libportaudio_static.a ..\..\lib
cd ..\..\
rmdir /Q /S portaudio
del pa_stable_v190600_20161030.tar

7z x freetype-2.9.tar.gz
if %errorlevel% neq 0 exit /b %errorlevel%
7z x freetype-2.9.tar
cd freetype-2.9
mkdir build
cd build
cmake .. -G"MinGW Makefiles" -DBUILD_SHARED_LIBS=OFF
cmake --build .
if %errorlevel% neq 0 exit /b %errorlevel%
xcopy ..\include ..\..\include /s /e
copy libfreetype.a ..\..\lib
cd ..\..
rmdir /Q /S freetype-2.9
del freetype-2.9.tar

