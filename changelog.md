@page changelog

small3d changes per version
===========================

v1.62 2018-10-18

 - No more "window not responding" messages in Linux during unit tests.
 - On MacOS, the window used during unit testing is now visible.
 - Another GLFW update.

v1.61 2018-09-27

 - Improved documentation generation procedure.
 - Using the latest GLFW snapshot on MacOS.	

v1.60 2018-09-15

- OpenGL v2.1 support dropped. small3d now only supports OpenGL v3.3 Core
  Profile, which in turn is now supported by a wide variety of operating systems
  versions. This allows for the same rendering code and shaders to be used
  accross Windows, MacOS and Linux. The shaders have also been brought to a
  state that would allow them to be easily upgraded to GLSL v4.5 and prepared
  for compilation for Vulkan, should I ever decide to move in that direction.
  
- Having mentioned the point above, here is a note on Vulkan and not really
  a small3d change for the moment: I have already prepared the necessary Vulkan
  code to migrate small3d to it and feel reasonably comfortable with this API.
  However, I am still not 100% convinced that that is the way to go. Even though
  I have not seen this being widely discussed online, I have personally witnessed
  Vulkan either not be supported and Vulkan drivers present serious glitches,
  like system freezes on various machines I have access to. So my layman's,
  totally humble conclusion which I would totally understand if better educated
  people than myself might disagree with is this: Vulkan is not that good yet.
  I am also slowly abandoning the idea of incorporating Metal support. The
  Vulkan-over-Metal adaptor (let's say) that is MoltenVK which has now been open
  sourced seems to work pretty well on MacOS. So IF OpenGL really does stop
  working on MacOS someday, it would probably be a good reason to migrate
  only to Vulkan, assuming that the latter has gotten a bit more widely adopted
  and its drivers a bit more stable by then. This would permit small3d to
  continue to have a single rendering codebase for all supported operating
  systems. But even though, as I have already mentioned, I have spent a good
  amount of time to be ready for it, that day has not yet arrived. 

v1.51 2018-06-06

- History reset. Changes that have taken place up to this point have now
  endured for so long that they are considered small3d's initial state.
