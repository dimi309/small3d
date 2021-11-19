file(GLOB SOURCE_FILES *.cpp)
list(REMOVE_ITEM SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/unit_tests.cpp)
list(REMOVE_ITEM SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/OctPyramid.cpp)

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

if(IOS)
  list(APPEND HEADER_FILES ${CMAKE_SOURCE_DIR}/deps/lib/interop.m)
endif()

add_library(small3d ${SOURCE_FILES} ${HEADER_FILES})

# When using OpenGL include the OpenGL Renderer.hpp directory first
# so that OpenGL Renderer.hpp takes precedence over Vulkan Renderer.hpp
if(SMALL3D_OPENGL)
  include_directories(${CMAKE_SOURCE_DIR}/opengl33/include/small3d)
endif()

include_directories(${Vulkan_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/include/small3d)

if(NOT DEFINED ANDROID_PLATFORM AND NOT IOS) # PC
  include_directories(
    ${PNG_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
    ${OGG_INCLUDE_DIRS}
    ${VORBIS_INCLUDE_DIR}
    ${FREETYPE_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIRS}
    ${PORTAUDIO_INCLUDE_DIRS}
    )
elseif(DEFINED ANDROID_PLATFORM) # Android
  target_include_directories(small3d PUBLIC
    ${CMAKE_SOURCE_DIR}/deps/include
    ${Vulkan_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIRS}
    )
elseif(IOS) #iOS
  target_include_directories(small3d PUBLIC
    ${CMAKE_SOURCE_DIR}/deps/include
    )
endif()

if(NOT SMALL3D_OPENGL AND NOT DEFINED ANDROID_PLATFORM AND NOT IOS) # PC, Vulkan
  target_link_libraries(small3d PUBLIC
    ${GLFW_LIBRARIES}
    ${Vulkan_LIBRARIES}
    ${PNG_LIBRARIES}
    ${VORBIS_LIBRARIES}
    ${OGG_LIBRARIES}
    ${PORTAUDIO_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    )
elseif(SMALL3D_OPENGL) # PC, OpenGL
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
elseif(DEFINED ANDROID_PLATFORM) # Android

  add_library(native_app_glue STATIC
    ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
  target_include_directories(native_app_glue PUBLIC
    ${ANDROID_NDK}/sources/android/native_app_glue)
  
  target_link_libraries(small3d
    android
    native_app_glue
    aaudio
    ${log-lib}
    ${Vulkan_LIBRARIES}
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/lib/${ANDROID_ABI}/libpng.a
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/lib/${ANDROID_ABI}/libvorbis.a
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/lib/${ANDROID_ABI}/libvorbisfile.a
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/lib/${ANDROID_ABI}/libogg.a
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/lib/${ANDROID_ABI}/libfreetype.a
    ${ZLIB_LIBRARIES})
elseif(IOS)
  target_link_libraries(small3d
    ${CMAKE_SOURCE_DIR}/deps/lib/libbz2.a
    ${CMAKE_SOURCE_DIR}/deps/lib/libfreetype.a
    ${CMAKE_SOURCE_DIR}/deps/lib/libMoltenVK.a
    ${CMAKE_SOURCE_DIR}/deps/lib/libogg.a
    ${CMAKE_SOURCE_DIR}/deps/lib/libpng.a
    ${CMAKE_SOURCE_DIR}/deps/lib/libvorbis.a
    ${CMAKE_SOURCE_DIR}/deps/lib/libvorbisfile.a
    ${CMAKE_SOURCE_DIR}/deps/lib/libz.a
    OpenAL.framework
    )
endif()

if(NOT DEFINED ANDROID_PLATFORM AND NOT IOS) # PC
  # on windows bzip2 is required for gcc version >= 10, otherwise a freetype associated
  # link error is produced
  if(UNIX OR (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" AND NOT
	CMAKE_CXX_COMPILER_VERSION VERSION_LESS 10))
    include_directories(${BZIP2_INCLUDE_DIRS})
    target_link_libraries(small3d PUBLIC ${BZIP2_LIBRARIES})
  endif()
  
  if(WIN32) # Windows
    target_link_libraries(small3d PUBLIC winmm)
  endif()
  
  if(UNIX AND NOT APPLE) # Linux
    target_link_libraries(small3d PUBLIC m pthread rt asound X11 dl)
  endif()

  # Unit testing

  add_executable(unittests unit_tests.cpp OctPyramid.hpp OctPyramid.cpp)

  # When using OpenGL include the OpenGL Renderer.hpp directory first
  # so that OpenGL Renderer.hpp takes precedence over Vulkan Renderer.hpp
  if(SMALL3D_OPENGL)
    target_include_directories(unittests PUBLIC
      ${CMAKE_SOURCE_DIR}/opengl33/include)
  endif()
  target_include_directories(unittests PUBLIC
      ${CMAKE_SOURCE_DIR}/include)

  target_link_libraries(unittests PUBLIC small3d)

  if(MSVC) # Visual Studio
    set_target_properties(unittests PROPERTIES LINK_FLAGS_DEBUG
      "-NODEFAULTLIB:LIBCMTD")
    set_target_properties(unittests PROPERTIES LINK_FLAGS_RELEASE
      "-NODEFAULTLIB:LIBCMT")
    
    set_target_properties(unittests PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY
      "${small3d_BINARY_DIR}/bin")
  endif()

  if(APPLE) # MacOS
    set_target_properties(unittests PROPERTIES LINK_FLAGS "-framework \
AudioUnit -framework AudioToolbox -framework CoreAudio -framework Cocoa \
-framework IOKit -framework CoreVideo")
  endif()


endif()
