# small3d

[![Build status](https://ci.appveyor.com/api/projects/status/qpm3qekslivm3kjb?svg=true)](https://ci.appveyor.com/project/dimi309/small3d)

[[Source Code]](https://github.com/dimi309/small3d) [[API Documentation]](https://dimi309.github.io/small3d)

Minimalistic, open source library for making 3D games in C++

## Sample games

- [Gloom](https://github.com/dimi309/gloom) (open source)

- [Islet Hell](https://store.steampowered.com/app/2069750/Islet_Hell/) (commercially released, also available for [Android](https://play.google.com/store/apps/details?id=dimi309.islethelladroid) and [Apple iOS](https://apps.apple.com/us/app/islet-hell/id1631875184))

# Features

- 3D models loaded from glTF (glb) files
- Other customised meshes and shapes
- Animation
- Texture mapping
- Gouraud shading
- Image rendering
- Font rendering
- Sound
- Collision detection
- Shadow mapping

# Supported platforms

- Windows (OpenGL)
- MacOS (OpenGL)
- Linux (OpenGL - Tested on Debian, Ubuntu and Fedora) 
- Android (OpenGL ES 2.0)
- iOS (OpenGL ES 2.0 - Extremely backwards compatible. It can run on iOS 9.3 and Android 4.2)

---
**NOTE**

I am aware that OpenGL and OpenGL ES have been deprecated by Apple in favour of 
Metal. This was one of the reasons [I was also maintaining a Vulkan renderer in the past](https://github.com/dimi309/small3d/releases/tag/1.8015.last.vulkan), 
which was also running on Apple devices via MoltenVK. I was hoping that, in the 
near future, I could use Vulkan exclusively across all platforms. However, 
given the many glitches I have encountered with Vulkan drivers on many devices, 
I have decided to remove the Vulkan renderer from small3d for now. Do not worry;
despite the deprecation, OpenGL and OpenGL ES still work just fine on Apple. I 
will reconsider implementing a Vulkan renderer or a Metal renderer in the future,
or even a DirectX renderer (why not) if OpenGL ever does indeed get discontinued 
on any platform small3d currently supports. In the meantime, my priority is that
games made with small3d run on as many devices as possible and not that I use 
the latest rendering technologies.

---

# Tutorial

https://www.gamedev.net/tutorials/programming/engines-and-middleware/small3d-tutorial-r5655/

# Building and deploying

## Prerequisites

The following need to be installed, with the relevant environment variables
and tools accessible via the command line:

- Some compiler, gcc, Visual Studio, clang, etc.
- 7zip (only on Windows)
- CMake
- Android Studio if building for Android
- Xcode if building for iOS

## Deploying for PC

Run the build script which is suitable to your platform from the `scripts` 
directory (`build-vs.bat`, `build-mingw.bat` or `build.sh`).
	
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

You can also deploy small3d using [conan](https://conan.io). The conan package is 
provided in a [separate repository](https://github.com/dimi309/small3d-conan).

## Deploying small3d for mobile

You can build small3d for mobile platforms by executing either `build-android` 
or  `build-ios` from the `scripts` directory. Then you can use the test projects 
in the `android`, `ios` and `ios-opengles` directories to check if everything works, 
or as a starting point for your own projects.

Note that, while with the desktop edition of small3d I use GLFW for windowing 
functionalities and I/O, on mobile I access the native infrastructure directly.

## Boilerplate Project

If you would like to start with an empty project that has the basic game loop 
and input already set up, you are looking for this:

https://github.com/dimi309/small3d-boilerplate

# Referenced libraries

All small3d dependencies are distributed together with its source code. 
They can be built by executing a single script (see "Building", above).

- glew if building for PC
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
