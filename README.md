# small3d

[![Build status](https://ci.appveyor.com/api/projects/status/qpm3qekslivm3kjb?svg=true)](https://ci.appveyor.com/project/dimi309/small3d)

[[Source Code]](https://github.com/dimi309/small3d) [[API Documentation]](https://dimi309.github.io/small3d)

Minimalistic, open source library for making 3D games in C++

## Sample games

- [Gloom](https://github.com/dimi309/gloom) (open source)

- [Islet Hell](https://store.steampowered.com/app/2069750/Islet_Hell/) (once commercially released, now free of charge, also available for [Android](https://play.google.com/store/apps/details?id=dimi309.islethelladroid) and [Apple iOS](https://apps.apple.com/us/app/islet-hell/id1631875184))

# Features

- 3D models loaded from glTF (glb files), Wavefront (obj files) or from a native format
- Other customised meshes and shapes
- Texture mapping
- Gouraud shading
- Shadow mapping
- Image rendering
- Font rendering
- Sound (ogg and native files)
- Collision detection
- Frame-based animation
- Skeletal animation (only based on rigs read from glb files; animation linked directly to models without bones/joints is ignored)

# Partly humorous list of things NOT supported

- Scenes - no Scene data structure; just SceneObjects rendered in an infinite space
- Materials
- Lighting other than Gouraud shading
- Ray tracing
- ECS
- AI
- Crypto

The purpose of this project is to provide a minimal set of features, allowing
C++ developers to build cross-platform games based on a single code-base.
Supporting as many platforms as possible while minimising the amount of code 
and time needed for maintenance is favoured over richness of features and using
cutting edge methods and technologies.

# Supported platforms

- Windows (OpenGL)
- MacOS (OpenGL)
- Linux (OpenGL - Tested on Debian, Ubuntu, Fedora and Arch) 
- FreeBSD (OpenGL)
- Android (OpenGL ES 2.0)
- iOS (OpenGL ES 2.0) 

This library is extremely backwards compatible. It can run on iOS 9.3 and 
Android 4.2.

*I am aware that OpenGL and OpenGL ES have been deprecated by Apple in favour of 
Metal. This was one of the reasons [I was also maintaining a Vulkan renderer in the past](https://github.com/dimi309/small3d/releases/tag/1.8015.last.vulkan), 
which was also running on Apple devices via MoltenVK. I was hoping that, in the 
near future, I could use Vulkan exclusively across all platforms. However, 
given the many glitches I have encountered with Vulkan drivers on many devices, 
I have decided to remove the Vulkan renderer for now. In any case, OpenGL and 
OpenGL ES still work just fine on Apple. I will reconsider implementing a Vulkan 
renderer or a Metal renderer in the future, or even a DirectX renderer (why not) 
if OpenGL ever does indeed get discontinued on any of the currently supported 
platforms. My priority is always that games made with this library run on as 
many devices as possible and that the codebase remains small - pun intended :)*

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

## Deploying small3d for PC

Run the build script which is suitable to your platform from the `scripts` 
directory (`build-vs.bat`, `build-mingw.bat` or `build.sh`).
	
Then, the unit tests are executed by running `unittests` binary from `build/bin`.

If any of this fails or you would simply like to restart the building
procedure, the best way to clean the repository is by using git:

	git clean -fdx

For building your own project, you need:

- The header files from the `build/include` directory
- The libraries from the `build/lib` directory 
- The shaders from `build/shaders` directory
- The `small3d/cmake` directory if you will be using cmake

You can also deploy using [conan](https://conan.io). The conan package is 
provided in a [separate repository](https://github.com/dimi309/small3d-conan).

## Deploying small3d for mobile

Builds for mobile platforms can be performed by executing either `build-android` 
or  `build-ios` from the `scripts` directory. You can then use the test projects 
in the `android`, `ios` and `ios-opengles` directories to check if everything 
works, or as a starting point for your own projects.

If the Android project produces an NDK or SDK related error when opened in 
Android Studio, just close it without exiting Android Studio and open it again.

Also on Android, while a game or the unit tests are running, the error 
`.../GL2Encoder.cpp:s_glGetBufferParameteriv:3386 GL error 0x502` might appear
in the log. You can ignore it. It is produced when glGetBufferParameteriv is 
called in the Renderer to check if a model has already been copied to the GPU 
and the model is not found (as expected when it has been newly loaded).

Note that, while on the PC edition I use GLFW for windowing functionalities and 
I/O, on mobile I access the native infrastructure directly.

## Boilerplate Project

If you would like to start with an empty project that has the basic game loop 
and input already set up, you are looking for this:

https://github.com/dimi309/small3d-boilerplate

It is highly recommended to use this boilerplate if you intend to port your
game to Android and / or iOS. While it is pretty straightforward to get this 
library working for the desktop and you might prefer the freedom and flexibility 
of working with a project set up from scratch, mobile platforms have many
specificities and quirks to take care of, so the boilerplate can save you a lot 
of time in that respect.

# Referenced libraries

- glew when building for PC
- glfw 
- glm 
- png
- zlib
- ogg
- vorbis
- portaudio
- freetype
- bzip
- cereal
- oboe when building for Android

All dependencies *source code* repositories (not binaries) are distributed
in this same repository (in the `deps` directory). They can be built by 
executing a single script (see "Building", above).
