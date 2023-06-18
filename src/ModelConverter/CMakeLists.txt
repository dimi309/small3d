add_executable(s3dmc model_converter.cpp)

target_link_libraries(s3dmc PRIVATE small3d)
if(MSVC) # Visual Studio
  set_target_properties(s3dmc PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY
    "${small3d_BINARY_DIR}/bin")
endif()
