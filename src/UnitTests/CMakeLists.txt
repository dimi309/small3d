add_executable(unittests UnitTests.hpp OctPyramid.cpp
  OctPyramid.hpp unit_tests.cpp UnitTests.cpp)

target_link_libraries(unittests PRIVATE small3d)
if(MSVC) # Visual Studio
  set_target_properties(unittests PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY
    "${small3d_BINARY_DIR}/bin")
endif()
