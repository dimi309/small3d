/*! \page vs Setting up small3d with Visual Studio

The `scripts/build-vs.bat` file with the relevant parameters (`Debug` or 
`Release` and, optionally, `opengl` if you prefer not to use Vulkan) will
create and build a Visual Studio based library and unit tests binary. Instead 
of just executing the binary, the solution can be opened in Visual Studio to be 
run, further developed and/or debugged from there.

There are two things to watch out for. First of all, when the Visual Studio
solution is first opened, the `ALL_BUILD` project is selected as the startup
project. This will not allow the unit tests to run when executing. It can
be corrected by right-clicking on the `unittests` project and then clicking
on `Set as Startup Project` on the context menu that appears.

Also, more often than not, the working directory of the unit tests executable
is not set up correctly. This can be verified by clicking on the `unittests`
project first, so that it is selected and then selecting `Properties` from 
the Visual Studio `Project` menu (or just right-clicking on `unittests` and
selecting `Properties` from there). 
Then, under `Configuration Properties > Debugging`, check the `Working
Directory` setting. it should be the `bin` directory under the `build`
folder created by `scripts/build-vs.bat`. If not, it needs to be set to
that.

The information above is also relevant when creating your own games and
applications with small3d.
