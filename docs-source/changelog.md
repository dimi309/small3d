/*! \page changelog changes per version

v1.801 (still open)

- VS build scripts switched to build with VS 2022 by default. In order to 
  go back to VS 2019 just change the VSCONFIG variables in 
  `srcipts/build-vs.bat` and `deps/scripts/prepare-vs.bat` to 
  `-G"Visual Studio 16 2019" -A x64`
- Stability improvements, especially for sound on Android and iOS.
- [BREAKING] SceneObject::setRotation(mat4x4) has been renamed to 
  setTransformation.
- Always loading the khronos validation layer on Android, even in release
  builds. This is done as a workaround to a Qualcomm Adreno driver bug, 
  which makes the vertex shader compilation crash when using Vulkan in 
  release builds on some devices.
- [BREAKING]  Renderer::setCameraRotation(const mat4x4&)
  has been renamed to setCameraTransformation(const mat4x4&).
- Added OpenGL ES 2.0 support for Android.
- Added Android and iOS test projects to the repository.

v1.800 2022-01-03

- [BREAKING] Rotations are now stored and managed as transformations. Euler
  angles (x, y, z rotation) support remains but with a modified interface.
- [BREAKING] SceneObject.offset has been renamed to SceneObject.position.
- [BREAKING] SceneObject.boundingBoxSet is now private. It is still possible
  to retrieve the bounding boxes as Models, using 
  SceneObject.getBoundingBoxSetModels, and the bounding box set extremes using 
  SceneObject.getBoundingBoxSetExtremes.
- SceneObject.boundingBoxSet and SceneObject.models have been converted
  to smart pointers to allow for the copy constructor to create scene
  objects that reuse existing models.
- Bounding boxes now take a scaling parameter, and the SceneObject
  scales the automatically generated bounding boxes to a Model's 
  initial scale.
- Added Model-based SceneObject constructor.
- Maximum number of objects that can be rendered per frame is now
  increased automatically each time that number is exceeded.

v1.709 2021-10-16

- Default light direction adjusted to match the perspective matrix correction
  of v1.708.
- [BREAKING] Removed custom-made intToStr and floatToStr functions. Using 
  std::to_string instead.
- Added glTF support.
- [BREAKING] Bounding boxes are now be generated automatically, instead of 
  being loaded from a file.
- [BREAKING] GetTokens is now just a private function of WavefrontFile. It
  cannot be used externally.
- [BREAKING] Models are no longer loaded with filenames as parameters. The
  appropriate file parsing object has to be passed instead.
- Scaling 3D models now also supported.
- Upgraded portaudio to stable v190700.
- Upgraded freetype to version 2.11.0.
- [BREAKING] Removed clearScreen command. Clearing the screen now happens
  automatically when swapBuffers is called.

v1.708 2020-12-25

- Elimination of compiler warnings and other code improvements.
- Upgraded to the latest versions of glfw, glm and freetype.
- Scripts now provided for preparing dependencies and building small3d on all 
  supported platforms.
- The README has been greatly shortened since a lot of the information that
  was there is now unnecessary. It can be found in the build scripts and
  it actually *runs* :)
- My perspective matrix was (mistakenly) row-major all this time. After
  converting it to column-major, I decided to just use glm::perspective 
  instead, since it is in effect the same thing and makes the code look
  simpler.

v1.707 2020-12-01

- [BREAKING] Improved collision detection.
- Merged OpenGL and Vulkan codebases. OpenGL can be optionally selected
  instead of Vulkan when configuring with CMake.
- Improved dependency preparation scripts.	
- Build scripts now provided for Android and iOS.

v1.706 2020-05-21

- The slowdown on MacOS in full screen mode has now been corrected and full
  screen mode has been re-enabled on MacOS.

- [BREAKING] Corrected coordinate system handedness (right handed 
  implementation, emulating OpenGL).

v1.705 2019-11-19

- Window resizing is now supported.

v1.704 2019-11-06

- Corrections for Android and iOS

v1.703 2019-11-02

- [BREAKING] Eliminated Renderer write function and all the renderRectangle
  functions as they were too resource hungry.
  
- Modified Renderer generateTexture function to contain a confirmation
  flag for replacing an existing texture if it has the same name as
  the texture being generated.
  
- Added some convenient render functions to Renderer, for rendering models
  without a given offset nor rotation (defaulting to 0).

v1.702 2019-10-26

- Introduced noCache flag for rendered text, allowing for the enclosing
  texture to be replaced when a new texture is created.

v1.701 2019-10-19

- Rationalised use of Vulkan synchronisation objects.

v1.700 2019-10-10

- Migrated to Vulkan

v1.697 2019-05-28

- README improvements

v1.696 2019-05-26

- Upgraded glfw to released v3.3.

v1.695 2019-04-28

- Stopped using Google Test.
- Improved dependency preparation scripts.

v1.694 2019-04-19

- Reorganisation and improvement of shader logic.

v1.693 2019-03-17

- Corrected google test related bug in VS Debug build.

v1.692 2019-01-19

- Removed all secure CRT functions from Visual Studio build.

v1.691 2018-12-11

 - Model now has a default constructor.

v1.69 2018-12-10

 - Now using google test v1.8.1.

v1.68 2018-12-09

 - Lower case characters like p and q were not being rendered entirely.
   This has now been corrected.
 - Upgraded FreeType to v2.9.1.
 - Bug fixes.

v1.67 2018-11-29

 - Added a generateTexture function to the Renderer, which creates a
   texture with some given text rendered within it.

v1.66 2018-11-16

 - Corrected texture mapping issue that occured when rendering textured 
   rectangles using the perspective shaders.

v1.65 2018-10-20

 - This project is now called the small3d framework, instead of the
   small3d game engine.
 - Documentation reorganisation.
	
v1.64 2018-10-18

 - Corrected blank window issue on Mojave (when not in full screen
   mode, nothing was being rendered until the game window was moved).

v1.63 2018-10-08

 - No more "window not responding" messages in Linux during unit tests.
 - On MacOS, the window used during unit testing is now visible.
 - Another GLFW update.

v1.62 2018-09-27

 - Improved documentation generation procedure.
 - Using the latest GLFW snapshot on MacOS.	

v1.60 2018-09-15

- OpenGL v2.1 support dropped. small3d now only supports OpenGL v3.3 Core
  Profile.

v1.51 2018-06-06

- History reset. Changes that have taken place up to this point have now
  endured for so long that they are considered small3d's initial state.
