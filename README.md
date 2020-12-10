# small3d

[[Source Code]](https://github.com/dimi309/small3d) [[API Documentation]](https://dimi309.github.io/small3d)

Minimalistic, open source library for making 3D games in C++, with
Vulkan and OpenGL support

## Sample games

- [Avoid the Bug](https://github.com/dimi309/avoidthebug)
- [Chase the Goat](https://github.com/dimi309/chasethegoat)
- [Frog Remixed](https://github.com/dimi309/frogremixed)
- [Gloom](https://github.com/dimi309/gloom)

## Features

- Loading & rendering Wavefront models
- Frame-based animation
- Texture mapping
- Gouraud shading
- Image rendering
- Font rendering
- Sound
- Collision detection

## Supported platforms

- Windows
- MacOS
- Linux (tested on Debian, Ubuntu and Fedora)
- Android
- iOS

## Building prerequisites

The following need to be installed, with the relevant environment variables
and tools accessible via the command line:

- Some compiler, gcc, Visual Studio, clang, etc.
- Vulkan SKD (if it will be used)
- 7zip (only on Windows)
- CMake

## Building

- Run the dependency preparation script which is suitable to your platform
  from the `deps` directory (`prepare-vs.bat`, `prepare-mingw.bat` or 
  `prepare.sh`).
- Run the build script which is suitable to your platform from the root 
  directory (`build-vs.bat`, `build-mingw.bat` or `build.sh`)
	
Then, the unit tests can be run via the `unittests` binary from `build/bin`.

If any of this fails or you would simply like to restart the building
procedure, the best way to clean the small3d directories is by using git:

	git clean -fdx

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

## Referenced libraries

All small3d dependencies, apart from the Vulkan SDK, are distributed together 
with its source code. They can be built by executing a single script 
(see "Building", above).

- Vulkan SDK, or glew if building for OpenGL
- glfw 
- glm 
- png
- zlib
- ogg
- vorbis
- portaudio
- freetype
- bzip
