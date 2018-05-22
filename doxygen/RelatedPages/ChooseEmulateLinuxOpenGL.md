Choosing and emulating OpenGL versions on Linux
===============================================

One matter that required attention during the early days of [small3d](https://github.com/dimi309/small3d)'s development was selecting an OpenGL version for the engine and making sure it works.

I decided to program small3d for OpenGL 3.3 and make it fall back to OpenGL 2.1 on systems that do not support the later API. Version 3.3 was widespread among new Windows and OSX systems and was considered to be pretty modern and a good foundation for understanding future versions. small3d is, if nothing else, a learning exercise in any case. I reached the conclusion that v2.1 was the lowest common denominator between older machines, especially those running Linux, since it was supported, by[Mesa](http://www.mesa3d.org/intro.html) software emulation at least, on my Debian system. Of course, when querying the supported version like this:

	glxinfo | grep Open

I would get OpenGL v1.4 returned. But when running this:

	LIBGL_ALWAYS_SOFTWARE=1 glxinfo | grep Open

I would indeed receive a result indicating OpenGL 2.1 support. The *LIBGL_ALWAYS_SOFTWARE* variable makes programs that run with it set to always use software rendering. So executing small3d's sample game like this:

	LIBGL_ALWAYS_SOFTWARE=1 ./samplegame
	
did indeed allow it to run on OpenGL 2.1 and use the corresponding shaders.

Now that things have evolved, my Debian does support v2.1 without emulation. However, this little trick is still useful. My current Linux box is even older than the one I had back then and, when running the sample game on it, the background looks a little messy. This does not happen on Windows or Apple machines. Still, I thought that perhaps I was not covering some special condition which causes the problem. I was about to spend time trying to solve it. But then I executed the game with the *LIBGL_ALWAYS_SOFTWARE* variable set, it switched to software emulation, and everything was looking fine. So I reached the conclusion that it is either the hardware driver or my very old GPU that is defective and decided not to worry about it, as far as correcting my code is concerned.
