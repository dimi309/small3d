set VSCONFIG=Visual Studio 15 2017 Win64
set BUILDTYPE=Release

7z x googletest-release-1.8.1.tar.gz
if %errorlevel% neq 0 exit /b %errorlevel%
7z x googletest-release-1.8.1.tar
cd googletest-release-1.8.1
mkdir build
cd build
cmake .. -G"%VSCONFIG%" -DBUILD_GMOCK=OFF -Dgtest_disable_pthreads=ON
cmake --build . --config %BUILDTYPE%
if %errorlevel% neq 0 exit /b %errorlevel%
xcopy ..\googletest\include\gtest ..\..\include\gtest /i /s
copy googletest\%BUILDTYPE%\gtest.lib ..\..\lib
copy googletest\%BUILDTYPE%\gtest_main.lib ..\..\lib\
for /r %%a in (*.pdb) do @copy /y "%%a" ..\..\bin
cd ..\..
rmdir /Q /S googletest-release-1.8.1
del googletest-release-1.8.1.tar
del pax_global_header
