/*! \page xcode Setting up small3d with Xcode

On MacOS, the small3d unit tests and sample game can be built from the command
line, just like on any other platform. However you might want to create an Xcode 
project, in order to use the Xcode debugger for example.

In this case, as a first step, instead of the `scripts/build.sh` script, 
the `deps/prapare.sh` has to be launched. Then, small3d together with its 
unit tests can be configured and built for Xcode as follows (execute from the 
main directory):

	mkdir build
	cd build
	cmake .. -G"Xcode"
	cmake --build .
	
The created project, `build/small3d.xcodeproj` can then be opened in Xcode.

There are some settings that need to be taken care of before proceeding to run
and debug the unit tests though. From the Project Navigator,
click on `small3d`, and then on the `unittests` target. Select `Build Settings`
and scroll down to `User-Defined`. There, the `CONFIGURATION_BUILD_DIR` variable
needs to be set to the `bin` directory in all cases,
for example `/Users/me/Source/small3d/build/bin/`, rather than the
differentiated directories it is set to by default, e.g. `bin/Debug`, 
`bin/Release`, etc.


