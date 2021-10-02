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

if(IOS)
  list(APPEND HEADER_FILES ${CMAKE_SOURCE_DIR}/deps/lib/interop.m)
endif()

add_library(small3d ${SOURCE_FILES} ${HEADER_FILES})

if(NOT SMALL3D_OPENGL)
  include_directories(${Vulkan_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/include/small3d)
else()
  include_directories(${OPENGL_INCLUDE_DIR}
    ${CMAKE_BINARY_DIR}/include/small3d) # because we don't want to include the Vulkan-based Renderer.hpp
  message(WARNING "ATTENTION: You cannot just edit small3d header files from an IDE like Visual Studio when using the OpenGL build. You need to also copy them to the build directory! (build/include/small3d)")
endif()

if(NOT DEFINED ANDROID_PLATFORM AND NOT IOS)
  include_directories(
    ${PNG_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
    ${OGG_INCLUDE_DIRS}
    ${VORBIS_INCLUDE_DIR}
    ${FREETYPE_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIRS}
    ${PORTAUDIO_INCLUDE_DIRS}
    )
elseif(DEFINED ANDROID_PLATFORM)
  target_include_directories(small3d PUBLIC
    ${CMAKE_SOURCE_DIR}/deps/include
    ${Vulkan_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIRS}
    )
elseif(IOS)
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
elseif(DEFINED ANDROID_PLATFORM)

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
  if(UNIX OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" AND NOT
	CMAKE_CXX_COMPILER_VERSION VERSION_LESS 10))
    include_directories(${BZIP2_INCLUDE_DIRS})
    target_link_libraries(small3d PUBLIC ${BZIP2_LIBRARIES})
  endif()
  
  if(WIN32)
    target_link_libraries(small3d PUBLIC winmm)
  endif()

  # Unit testing

  add_executable(unittests unit_tests.cpp)

  if(NOT SMALL3D_OPENGL)
    target_include_directories(unittests PUBLIC
      ${CMAKE_SOURCE_DIR}/include)
  else()
    target_include_directories(unittests PUBLIC
      ${CMAKE_BINARY_DIR}/include) # because we don't want to include the Vulkan-based Renderer.hpp
  endif()

  target_link_libraries(unittests PUBLIC small3d)


  if(MSVC)
    set_target_properties(unittests PROPERTIES LINK_FLAGS_DEBUG
      "-NODEFAULTLIB:LIBCMTD")
    set_target_properties(unittests PROPERTIES LINK_FLAGS_RELEASE
      "-NODEFAULTLIB:LIBCMT")
    
    set_target_properties(unittests PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY
      "${small3d_BINARY_DIR}/bin")
  endif()

  if(APPLE)
    set_target_properties(unittests PROPERTIES LINK_FLAGS "-framework \
AudioUnit -framework AudioToolbox -framework CoreAudio -framework Cocoa \
-framework IOKit -framework CoreVideo")
  endif()

  if(UNIX AND NOT APPLE)
    target_link_libraries(small3d PUBLIC m pthread dl rt X11)
  endif()

endif()
