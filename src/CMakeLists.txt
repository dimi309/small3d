file(GLOB SOURCE_FILES *.cpp)
list(REMOVE_ITEM SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/unit_tests.cpp)

file(GLOB HEADER_FILES ${CMAKE_SOURCE_DIR}/include/small3d/*.hpp)

if(NOT SMALL3D_OPENGL)
  list(APPEND SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/vkzos.c)
  list(APPEND HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/vkzos.h)
else()
  list(REMOVE_ITEM SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Renderer.cpp)
  list(APPEND SOURCE_FILES ${CMAKE_SOURCE_DIR}/opengl33/src/Renderer.cpp)
  list(REMOVE_ITEM HEADER_FILES  ${CMAKE_SOURCE_DIR}/include/small3d/Renderer.hpp)
  list(APPEND HEADER_FILES ${CMAKE_SOURCE_DIR}/opengl33/include/small3d/Renderer.hpp)
endif()

add_library(small3d ${SOURCE_FILES} ${HEADER_FILES})

include_directories(
  "${CMAKE_SOURCE_DIR}/include/small3d"
  ${GLFW_INCLUDE_DIRS}
  ${PNG_INCLUDE_DIRS}
  ${GLM_INCLUDE_DIRS}
  ${OGG_INCLUDE_DIRS}
  ${VORBIS_INCLUDE_DIR}
  ${PORTAUDIO_INCLUDE_DIRS}
  ${FREETYPE_INCLUDE_DIRS}
  )

if(NOT SMALL3D_OPENGL)
  include_directories(${Vulkan_INCLUDE_DIRS})
else()
  include_directories(${OPENGL_INCLUDE_DIR})
endif()

if(NOT SMALL3D_OPENGL)
  target_link_libraries(small3d PUBLIC
    ${GLFW_LIBRARIES}
    ${Vulkan_LIBRARIES}
    ${PNG_LIBRARIES}
    ${VORBIS_LIBRARIES}
    ${OGG_LIBRARIES}
    ${PORTAUDIO_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    )
else()
  target_link_libraries(small3d PUBLIC
    ${GLEW_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARIES}
    ${PNG_LIBRARIES}
    ${VORBIS_LIBRARIES}
    ${OGG_LIBRARIES}
    ${PORTAUDIO_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    )
endif()

if(UNIX)
  include_directories(${BZIP2_INCLUDE_DIRS})
  target_link_libraries(small3d PUBLIC ${BZIP2_LIBRARIES})
endif(UNIX)

if(WIN32)
  target_link_libraries(small3d PUBLIC winmm)
endif(WIN32)

# Unit testing

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
elseif(UNIX AND NOT SMALL3D_OPENGL)
  target_link_libraries(unittests PUBLIC "m")
endif(APPLE)
