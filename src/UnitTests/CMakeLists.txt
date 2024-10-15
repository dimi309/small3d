add_executable(unittests UnitTests.hpp unit_tests.cpp UnitTests.cpp)

target_link_libraries(unittests PRIVATE small3d)

if(MSVC) # Visual Studio

  # Without setting the link flags below, the following type of warning occurs:
  # LINK : warning LNK4098: defaultlib 'LIBCMTD' conflicts with use of other libs; use /NODEFAULTLIB:library ...
  #        ...\small3d\build\src\UnitTests\unittests.vcxproj]
  set_target_properties(unittests PROPERTIES LINK_FLAGS_DEBUG
    "-NODEFAULTLIB:LIBCMTD")
  set_target_properties(unittests PROPERTIES LINK_FLAGS_RELEASE
    "-NODEFAULTLIB:LIBCMT")

  # This is so that the program considers /bin its working directory, even
  # when launched with the debugger.
  set_target_properties(unittests PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY
    "${small3d_BINARY_DIR}/bin")
endif()
