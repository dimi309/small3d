@page an_update_after_all_these_years An update after all these years
@brief 2018-06-20

This is a note intended to let you know that small3d is still in active development. It has been a few years now since I started this project. My initial goal was to [learn how to program 3D games](https://www.gamedev.net/articles/programming/general-and-gameplay-programming/building-an-open-source-cross-platform-3d-game-with-c-opengl-and-glsl-from-the-ground-up-r3819). Later on, I [converted the code I had written during the process to a small 3D game engine](https://www.gamedev.net/articles/programming/general-and-gameplay-programming/a-rudimentary-3d-game-engine-built-with-c-opengl-and-glsl-r4176). I was using a C++ package manager at the time, called biicode, and then I migrated to the new and more awesome [conan](https://conan.io/), built by the same core team.

Since then I have spent a considerable amount of time ensuring that the engine and the sample games can be easily compiled on Windows, MacOS and Linux (Debian, Fedora and Ubuntu). I have improved the coding quality and corrected many errors.

Based on the feedback I have received over the years, I would say that this project has been a modest success. Some have compiled and modified the sample games, some have cloned the repository and starred it on GitHub and some have even created pull requests.

Below is a brief account of the latest changes and my thoughts for the future.

Quitting conan (the package manager)
------------------------------------
I am no longer using a package manager. I still think that conan is great and can considerably decrease the effort required to develop and maintain a project, *under certain circumstances*. In the case of this project, I have found that some users do not want to adopt such a technology. So I have now packaged all the required libraries together with small3d and created scripts that build everything for each targetted platform. This will also allow me spend time developing the engine without having to watch changes in the underlying package management technology or package configurations.

GitHub
------
In case you haven't noticed, I am no longer on GitHub. No, I am not #movingtogitlab and neither do I have a problem with any transactions that have recently taken place inside or outside the stock market. I do think however that carrying a public legacy of 1000+ commits (approximate number at the time I went out) is an overkill. I am now just releasing versions on Sourceforge.

Vulkan
------
I have learned how to use this thing and started preparing a library that will allow small3d to use it in addition to OpenGL. But I have decided to put that on hold for now. This new library is about 2K lines of code, and will increase the complexity of small3d, missing the whole point of keeping the game engine *small*. Also, I'm not sure what good it would do. Vulkan works on fewer machines than OpenGL does right now as far as I can tell. On MacOS it is running on a Metal layer. Also, the only platform on which OpenGL may stop working at some point seems to be the Mac. So it probably makes sense to look into incorporating Metal before Vulkan. But I will not be doing that either until the day when the engine stops working on MacOS.


