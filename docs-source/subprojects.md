/*! \page subprojects Developing a game in parallel with %small3d

These are the steps to add an existing small3d-based project to the %small3d
directory and link it with the %small3d library project/solution, so that it
can be debugged and modified in parallel with %small3d.

- Build %small3d but then delete the build folder. This will prepare all
  %small3d dependencies (in the deps folder).
- Copy the folder of the project to be added to the %small3d root folder
- Add the project folder name to the subdirs statement in small3d/CMakeLists.txt
- Remove everything from CMakeLists.txt at the root of the added project,
  only leaving these lines:
  
		project(projectname)
		file(COPY "resources" DESTINATION "${PROJECT_BINARY_DIR}/bin")
		subdirs(src)
  
- In the added project subfolder, edit src/CMakeLists.txt and replace
  reference variables to the %small3d library like ${SMALL3D_LIBRARY} by
  the simple string "small3d" (without quotes).
- Go to the root of the %small3d project and execute:

		mkdir build
		cd build
	
  and then the cmake configuration and build commands, e.g.:

		cmake .. -G"Visual Studio 17 2022"
		cmake --build . --config Debug

- Everything is now ready. If using Visual Studio you can find the solution
  in the build folder. The binaries for the %small3d unit tests and the added
  project will be in the usual location, build/bin.

- If using Visual Studio, you might need to open the properties of the added
  project, and set Configuration Properties > Debugging > Working Directory
  to the small3d/build/bin directory, because cmake may have added the
  project folder name before the /bin part, possibly causing the subproject
  executable to not be able to find required files like shaders and models.
  Also don't forget to set the added project as the startup project if
  you would like to run and debug it.

- Make sure that the resources folder contains the correct files for the
  project you would like to run, the added one or the %small3d unit tests.
  If resources between the two have the same names they may have
  overwritten each other or they may cause other problems, depending on
  the project added.
