A modified version of the freetype library is provided in addition
to the normal distributable, which defines FT_EXPORT( x ) as
extern x rather than __declspec( dllimport ) x in the
/include/freetype/config/ftconfig.h file. This was necessary for
compilation with Visual Studio.

Again for compiling with Visual Studio, a modified version of the
googletest library is also provided in addition to the normal
distributable, which is configured for less strict error checking.

Each dependency build script (prepare.bat, prepare-vs.bat and
prepare.sh) will use the version of the above mentioned libraries
that is required for the build for which it has been created. No
special configuration or parameter is required to that end.
