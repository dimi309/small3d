# small3d

[![Build status](https://ci.appveyor.com/api/projects/status/qpm3qekslivm3kjb?svg=true)](https://ci.appveyor.com/project/dimi309/small3d)

[[Source Code]](https://github.com/dimi309/small3d) [[API Documentation]](https://dimi309.github.io/small3d)

Minimalistic, open source library for making 3D games in C++

This library provides a sufficient level of game development functionalities 
for C++ developers to be able to build cross-platform games based on a single 
code-base. Supporting as many platforms as possible while minimising the amount 
of code and time needed for maintenance is favoured over richness of features 
and the use of cutting edge methods and technologies.

# Games 

## First little games

- Avoid the Bug

- Chase the Goat

- Frog Remixed

All of the above can be found here: https://github.com/dimi309/small3d-first-games

## More "evolved" titles

- [Gloom](https://github.com/dimi309/gloom) (open source)

- [Islet Hell](https://store.steampowered.com/app/2069750/Islet_Hell/) (once commercially released, now free of charge, also available for [Android](https://play.google.com/store/apps/details?id=dimi309.islethelladroid) and [Apple iOS](https://apps.apple.com/us/app/islet-hell/id1631875184))

# Non-game project

- [Serial Model Renderer](https://github.com/dimi309/serial-model-renderer), used to test the 3D model reading and rendering functionality of small3d

# Features

- 3D models loaded from glTF (glb files), Wavefront (obj files) 
  or from a native format
- Other customised meshes and shapes
- Texture mapping
- Gouraud shading
- Basic materials support (colour, transparency)
- Shadow mapping
- Image rendering
- Font rendering
- Sound (ogg and native files)
- Collision detection
- Frame-based animation (Wavefront)
- Skeletal animation (glTF)

# Limitations

- Non-PNG images are not supported.
- There are no scenes, just SceneObjects rendered in an infinite space.
- The 3D model parsers are far from feature-complete but pretty robust 
  nonetheless. The goal is not to be able to read a complete node structure
  from a gltf file for example, but to extract individual models and their
  animations, in order to use them in a game.

# Supported platforms

- Windows
- MacOS
- Linux (Tested on Debian, Ubuntu, Fedora and Arch) 
- FreeBSD
- Android
- iOS

Running on OpenGL on PC and OpenGL ES on mobile, this library is extremely 
backwards compatible. It can run on iOS 9.3 and Android 4.1.

Note: There used to be Vulkan support too, but it has been discontinued. Here is
an article on the reasons for this:

https://www.gamedev.net/blogs/entry/2275791-abandoning-vulkan/

And here is the last Vulkan commit before the Vulkan renderer got removed:

https://github.com/dimi309/small3d/releases/tag/1.8015.last.vulkan

Note that despite the official deprecation of OpenGL on Apple devices,
OpenGL and OpenGL ES still work just fine there. If and when that changes, 
I will implement a renderer in Metal or something.

# Tutorial

https://www.gamedev.net/tutorials/programming/engines-and-middleware/small3d-tutorial-r5655/

# Building and deploying

## Prerequisites

The following need to be installed, with the relevant environment variables
and tools accessible via the command line:

- Some compiler (e.g. gcc, Visual Studio, clang) with C++17 support recommended
  as a minimum.
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

The following dependencies' *source code* repositories (not binaries) are 
distributed in this same repository (in the `deps` directory). They can be built
by executing a single script (see "Building", above) and they can also be used 
directly in your application / game code.

- glew when building for PC
- glfw 
- glm 
- png
- zlib
- ogg
- vorbis
- portaudio
- freetype
- cereal
- oboe when building for Android


