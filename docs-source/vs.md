/*! \page vs Setting up small3d with Visual Studio

The `scripts/build-vs.bat` file with the relevant parameters (`Debug` or 
`Release` and, optionally, `opengl` if you prefer not to use Vulkan) will
create and build a Visual Studio based library and unit tests binary. Instead 
of just executing the binary, the solution `small3d.sln`, found in the 
`build` directory, can be opened in Visual Studio to be run, further developed
and/or debugged from there.

When the solution is first opened in Visual Studio, the `ALL_BUILD` project 
is selected as the startup project. This will not allow the unit tests to 
run when executing. It can be corrected by right-clicking on the `unittests` 
project and then clicking on `Set as Startup Project` on the context menu 
that appears.

