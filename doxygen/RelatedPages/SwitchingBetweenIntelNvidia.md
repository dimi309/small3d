Make sure you are using your NVIDIA chip on Linux
=================================================

In the early days of [small3d](https://github.com/dimi309/small3d)'s development, I was using a laptop which I was dual booting into Windows and Debian. The machine had been purchased with Windows pre-installed, and then I also installed the Linux instance. Even though the Windows system reported support for OpenGL 4.2, my Linux image reported OpenGL 3.0 when running glxinfo:

	glxinfo | grep Open
	
What was strange was that it mentioned an Intel VGA instead of the NVIDIA GeForce which I knew I had. The little detail that I was missing up to that point was that I had two GPUs.

NVIDIA supports [Optimus](http://www.nvidia.com/object/optimus_technology.html) technology. If you have a laptop, it could be the case that you have two GPUs too, basically a GeForce chip and an Intel chip. The latter is less capable graphics-wise but it consumes less energy, so Optimus is supposed to switch between the two, choosing GeForce for the more demanding, graphics-intensive tasks, like games or CAD applications, and Intel for the more mundane ones, like document or spreadsheet editing. In Windows the switch between the two is transparent to the user. However, if you are using Ubuntu for example, you may have to take a few extra steps.

First of all, to check whether or not you have two video adapters, you can run the following:

	lspci -v | grep VGA

If two entries are returned, one from Intel and one from NVIDIA, your machine is probably using Optimus. In my case, this lead to all programs running on the Intel GPU, regardless of whether or not they were a game or a word processor. I also found out that it is not possible (or not a good idea at least) to configure the entire system only use the GeForce chip. There are two packages that allow us to direct a specific executable to the NVIDIA GPU by prefixing it with a certain command. These are [Bumblebee](http://bumblebee-project.org/) and Primus. I have tried them both and they work well, but Primus runs on top of Bumblebee and it is recommended by the Bumblebee developers as the way to go (it forms part of the same project). On a Debian distribution for example, you can install them by executing:

	sudo apt-get install bumblebee primus
	
(Note that there are [more detailed instructions](http://bumblebee-project.org/install.html) provided for installation and use, depending on your configuration)

After having done so myself, when I would execute:

	primusrun glxinfo | grep OpenGL
	
or for plain Bumblebee:

	optirun glxinfo | grep OpenGL
	
The results indicated, in both cases, that the NVIDIA chip had been activated and that it supported OpenGL 4.2.

So I created a bash command file to run Eclipse (with which I was editing the game engine's code at the time) which contained:

	primusrun [path to my Eclipse]/eclipse
	
This allowed my whole development environment, including executables being debugged, to run on the NVIDIA GPU. GLEW also reported that OpenGL 3.3 was supported, so I could work with that version of the API.
