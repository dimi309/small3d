add_executable(s3dfc format_converter.cpp)

target_link_libraries(s3dfc PRIVATE small3d)
if(MSVC) # Visual Studio
  set_target_properties(s3dfc PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY
    "${small3d_BINARY_DIR}/bin")
endif()
