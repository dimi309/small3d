# small3d

[[Source Code]](https://github.com/dimi309/small3d) [[API Documentation]](https://dimi309.github.io/small3d)

Free, open source, minimalistic 3D framework for the programmer who 
would like to make games using a basic set of libraries (glfw, 
glm, png, zlib, ogg, vorbis, portaudio, freetype, bzip) and relying on
C++ to do the rest. It helps you by providing you with cross-platform rendering
functionality based on Vulkan. It can also be compiled with OpenGL. 

small3d can render Wavefront models, animate them as frames, map textures on 
them, provide some basic lighting (Gouraud shading) and also render images and
text.

A very easy to use Sound object is also provided that can play OGG files on all
supported platforms via a common interface. Basic collision detection has
also been implemented.

small3d works on Windows, MacOS, Linux, iOS and Android and supports Visual 
Studio, Xcode, gcc (even MinGW) and clang for compilation.

All small3d dependencies, apart from the Vulkan SDK, are distributed together 
with its source code. They can be built by executing a single script (see 
below).

## Sample games

- [Avoid the Bug](https://github.com/dimi309/avoidthebug)
- [Chase the Goat](https://github.com/dimi309/chasethegoat)
- [Frog Remixed](https://github.com/dimi309/frogremixed)
- [Gloom](https://github.com/dimi309/gloom)

## Building prerequisites

The following need to be installed, with the relevant environment variables
and tools accessible via the command line:

- Some compiler, gcc, Visual Studio, clang, etc.
- Vulkan SKD (if it will be used)
- 7zip
- CMake

## Building

- Run the dependency preparation script which is suitable to your platform
  from the `deps` directory (`prepare-vs.bat`, `prepare-mingw.bat` or 
  `prepare.sh`).
- Run the build script which is suitable to your platform from the root 
  directory (`build-vs.bat`, `build-mingw.bat` or `build.sh`)
	
Then, the unit tests can be run via the `unittests` binary from `build/bin`. 

For building your own project, you need:

- The header files from the `build/include` directory
- The libraries from the `build/lib` directory 
- The shaders from `build/shaders` directory

If you are using cmake, the modules in `small3d/cmake` can be useful. Check the 
`CMakeLists.txt` and `src/CMakeLists.txt` files for other configuration details 
(link flags, etc) that may also be required or useful.

## small3d on mobile

The scripts for preparing the dependencies of small3d for mobile are `prepare-android`
and `prepare-ios` for Android and iOS respectively, found in the `deps` directory. 
After executing one of those, you can build small3d for these platforms by executing
either `build-android` or `build-ios` from the main directory. One sample game, 
[Avoid the Bug](https://github.com/dimi309/avoidthebug) has been ported to
both [Android](https://github.com/dimi309/avoidthebug-android) and 
[iOS](https://github.com/dimi309/avoidthebug-ios). I use these projects as a basis
for mobile development. Note that, while with the desktop edition of small3d
I use GLFW for windowing functionalities and I/O, on mobile I access the native
infrastructure directly.
