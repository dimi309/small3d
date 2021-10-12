# small3d

[![Build status](https://ci.appveyor.com/api/projects/status/qpm3qekslivm3kjb?svg=true)](https://ci.appveyor.com/project/dimi309/small3d)

[[Source Code]](https://github.com/dimi309/small3d) [[API Documentation]](https://dimi309.github.io/small3d)

Minimalistic, open source library for making 3D games in C++, with
Vulkan and OpenGL support

## Sample games

- [Avoid the Bug](https://github.com/dimi309/small3d-samples/tree/master/avoidthebug)
- [Chase the Goat](https://github.com/dimi309/small3d-samples/tree/master/chasethegoat)
- [Frog Remixed](https://github.com/dimi309/small3d-samples/tree/master/frogremixed)
- [Gloom](https://github.com/dimi309/small3d-samples/tree/master/gloom)

## Tutorial

https://www.gamedev.net/tutorials/programming/engines-and-middleware/small3d-tutorial-r5655/

## Features

- 3D models loaded from glTF (glb) or Wavefront (obj) files
- Other customised meshes and shapes
- Animation
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
- Vulkan SDK (if it will be used)
- 7zip (only on Windows)
- CMake

## Building

Run the build script which is suitable to your platform from the scripts 
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

## Building and packaging with conan

To package small3d with [conan.io](https://conan.io), add the [Bincrafters repository](https://bincrafters.readthedocs.io/en/latest/using_packages.html#adding-the-bincrafters-repository-as-a-conan-remote):

    conan remote add bincrafters https://bincrafters.jfrog.io/artifactory/api/conan/public-conan
    conan config set general.revisions_enabled=1
 
Set up the small3d package locally:

    git clone https://github.com/dimi309/small3d
    cd small3d
	cd conan_io
    conan export . small3d/master@
	 
The package will be set up as `small3d/master` in your local conan repository.

## small3d on mobile

You can build small3d for mobile platforms by executing either `build-android` or 
`build-ios` from the scripts directory. One sample game, [Avoid the Bug](https://github.com/dimi309/small3d-samples/tree/master/avoidthebug)
has been ported to both [Android](https://github.com/dimi309/small3d-samples/tree/master/avoidthebug-android) 
and [iOS](https://github.com/dimi309/small3d-samples/tree/master/avoidthebug-ios). 
I use these projects as a basis for mobile development. Note that, while with 
the desktop edition of small3dI use GLFW for windowing functionalities and I/O,
on mobile I access the native infrastructure directly.

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
