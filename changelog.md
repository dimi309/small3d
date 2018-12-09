@page changelog

small3d changes per version
===========================

v1.68 (open)

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
