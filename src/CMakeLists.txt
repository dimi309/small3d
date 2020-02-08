file(GLOB SOURCE_FILES *.cpp)
list(REMOVE_ITEM SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/unit_tests.cpp)
list(APPEND SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/vkzos.c)
file(GLOB HEADER_FILES ${CMAKE_SOURCE_DIR}/include/small3d/*.hpp)
list(APPEND HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/vkzos.h)

add_library(small3d ${SOURCE_FILES} ${HEADER_FILES})

include_directories(
  "${CMAKE_SOURCE_DIR}/include/small3d"
  ${GLFW_INCLUDE_DIRS}
  ${Vulkan_INCLUDE_DIRS}
  ${PNG_INCLUDE_DIRS}
  ${GLM_INCLUDE_DIRS}
  ${OGG_INCLUDE_DIRS}
  ${VORBIS_INCLUDE_DIR}
  ${PORTAUDIO_INCLUDE_DIRS}
  ${FREETYPE_INCLUDE_DIRS}
  )

target_link_libraries(small3d PUBLIC
  ${GLFW_LIBRARIES}
  ${Vulkan_LIBRARIES}
  ${PNG_LIBRARIES}
  ${VORBIS_LIBRARIES}
  ${OGG_LIBRARIES}
  ${PORTAUDIO_LIBRARIES}
  ${FREETYPE_LIBRARIES}
  )

if(UNIX)
  include_directories(${BZIP2_INCLUDE_DIRS})
  target_link_libraries(small3d PUBLIC ${BZIP2_LIBRARIES})
endif(UNIX)

if(WIN32)
  target_link_libraries(small3d PUBLIC winmm)
endif(WIN32)

# Unit testing
if(BUILD_UNIT_TESTS)
  add_executable(unittests unit_tests.cpp)

  target_include_directories(unittests PUBLIC
    "${CMAKE_SOURCE_DIR}/include")

  target_link_libraries(unittests PUBLIC small3d)

  if(MSVC)
    set_target_properties(unittests PROPERTIES LINK_FLAGS_DEBUG
      "-NODEFAULTLIB:LIBCMTD")
    set_target_properties(unittests PROPERTIES LINK_FLAGS_RELEASE
      "-NODEFAULTLIB:LIBCMT")
    
    set_target_properties(unittests PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY
      "${small3d_BINARY_DIR}/bin")
  endif(MSVC)

  if(APPLE)
    set_target_properties(unittests PROPERTIES LINK_FLAGS "-framework \
AudioUnit -framework AudioToolbox -framework CoreAudio -framework Cocoa \
-framework IOKit -framework CoreVideo")
  elseif(UNIX)
    target_link_libraries(unittests PUBLIC "m")
  endif(APPLE)
endif(BUILD_UNIT_TESTS)

