For compiling with Visual Studio, a modified version of the
googletest library is provided in addition to the normal
distributable, which is configured for less strict error checking.

Each dependency build script (prepare.bat, prepare-vs.bat and
prepare.sh) will use the version of the above mentioned library
that is required for the build for which it has been created. No
special configuration or parameter is required to that end.
