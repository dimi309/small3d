add_executable(s3dfc format_converter.cpp)

target_link_libraries(s3dfc PRIVATE small3d)

if(MSVC) # Visual Studio

  # Without setting the link flags below, the following type of warning occurs:
  # LINK : warning LNK4098: defaultlib 'LIBCMTD' conflicts with use of other libs; use /NODEFAULTLIB:library ...
  #        ...\small3d\build\src\UnitTests\unittests.vcxproj]
  set_target_properties(s3dfc PROPERTIES LINK_FLAGS_DEBUG
    "-NODEFAULTLIB:LIBCMTD")
  set_target_properties(s3dfc PROPERTIES LINK_FLAGS_RELEASE
    "-NODEFAULTLIB:LIBCMT")

  # This is so that the program considers /bin its working directory, even
  # when launched with the debugger.
  set_target_properties(s3dfc PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY
    "${small3d_BINARY_DIR}/bin")
endif()


