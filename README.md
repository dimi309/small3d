# small3d

[![Build status](https://ci.appveyor.com/api/projects/status/qpm3qekslivm3kjb?svg=true)](https://ci.appveyor.com/project/dimi309/small3d)

[[Source Code]](https://github.com/dimi309/small3d) [[API Documentation]](https://dimi309.github.io/small3d)

Minimalistic, open source library for making 3D games in C++, with
Vulkan, OpenGL and OpenGL ES support.

## Sample games

- [Gloom](https://github.com/dimi309/gloom) (open source)

- [Islet Hell](https://store.steampowered.com/app/2069750/Islet_Hell/) (commercially released, also available for [Android](https://play.google.com/store/apps/details?id=dimi309.islethelladroid) and [Apple iOS](https://apps.apple.com/us/app/islet-hell/id1631875184))

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
- Shadow mapping

## Supported platforms

- Windows
- MacOS
- Linux (tested on Debian, Ubuntu and Fedora)
- Android
- iOS

(Extremely backwards compatible. It can run on iOS 9.3 and Android 4.2.)

## Building prerequisites

The following need to be installed, with the relevant environment variables
and tools accessible via the command line:

- Some compiler, gcc, Visual Studio, clang, etc.
- Vulkan SDK (if it will be used)
- 7zip (only on Windows)
- CMake

## Building and deploying

---
**NOTE**

Building and deployment can be carried out the "traditional" way, just 
with CMake and batch scripts, but deployment with conan.io is also
available. small3d has been recently migrated to conan 2.0 in that
respect. The Vulkan build does not yet work with conan (you can try it
but it will probably crash), but the OpenGL should be ok. You need to
deploy my variation of the portaudio package for conan as a prerequisite:

	git clone https://github.com/dimi309/portaudio-conan
	cd portaudio-conan
	conan export . --version=19.7.0

And of course, small3d too:

	git clone https://github.com/dimi309/small3d
	cd small3d
	conan export . --version=master
---

For "traditional" deployment, run the build script which is suitable to your platform 
from the scripts directory (`build-vs.bat`, `build-mingw.bat` or `build.sh`).
	
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

small3d runs on mobile devices using Vulkan and OpenGL ES. On Android, OpenGL ES 
is recommended because Vulkan still has some issues on quite a few devices.

Concerning iOS, it has to be mentioned that OpenGL ES has been deprecated by
Apple. It is not a bad idea to use it though. It still works and it will run
on older 32-bit devices for which the Vulkan build will not. Other than that, 
small3d runs fine with Vulkan on 64-bit iOS devices (using MoltenVK).

You can build small3d for mobile platforms by executing either `build-android` 
or  `build-ios` from the `scripts` directory. Then you can use the test projects 
in the `android`, `ios` and `ios-opengles` directories to check if everything works, 
or as a starting point for your own projects.

Note that, while with the desktop edition of small3d I use GLFW for windowing 
functionalities and I/O, on mobile I access the native infrastructure directly.

## Referenced libraries

All small3d dependencies, apart from the Vulkan SDK, are distributed together 
with its source code. They can be built by executing a single script 
(see "Building", above).

- Vulkan SDK, or glew if building with OpenGL for PC
- glfw 
- glm 
- png
- zlib
- ogg
- vorbis
- portaudio
- freetype
- bzip
- oboe if building for Android
