add_library(small3d BasePath.cpp BoundingBoxSet.cpp File.cpp GlbFile.cpp
  WavefrontFile.cpp BinaryFile.cpp Image.cpp Logger.cpp Model.cpp Renderer.cpp
  SceneObject.cpp  Sound.cpp Time.cpp ../include/small3d/SceneObject.hpp
  ../include/small3d/Sound.hpp ../include/small3d/Time.hpp
  ../include/small3d/BasePath.hpp ../include/small3d/BoundingBoxSet.hpp
  ../include/small3d/File.hpp ../include/small3d/GlbFile.hpp
  ../include/small3d/WavefrontFile.hpp ../include/small3d/BinaryFile.hpp
  ../include/small3d/Image.hpp ../include/small3d/Logger.hpp
  ../include/small3d/Model.hpp ../include/small3d/Renderer.hpp
  )

if(DEFINED ANDROID_PLATFORM)
  target_sources(small3d PRIVATE small3d_android.c)
endif()

target_include_directories(small3d PUBLIC ../include/small3d)

if(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
target_include_directories(small3d PUBLIC /usr/local/include)
endif()

if(NOT DEFINED ANDROID_PLATFORM AND NOT IOS) # PC
  target_include_directories(small3d PUBLIC
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
    ${ZLIB_INCLUDE_DIRS}
    )
  # The following is so that small3d_android.h can be included
  target_include_directories(small3d PUBLIC ${CMAKE_SOURCE_DIR}/src)
elseif(IOS) #iOS
  target_include_directories(small3d PUBLIC
    ${CMAKE_SOURCE_DIR}/deps/include
    )
endif()

if(DEFINED ANDROID_PLATFORM OR IOS)
  target_compile_definitions(small3d PUBLIC SMALL3D_OPENGLES)
endif()

if(WIN32 AND NOT DEFINED ANDROID_PLATFORM AND NOT IOS)
  target_compile_definitions(small3d PRIVATE GLEW_STATIC)
endif()

target_compile_definitions(small3d PRIVATE ${PNG_DEFINITIONS})

if(IOS)
  target_compile_definitions(small3d PUBLIC SMALL3D_IOS)
endif()

if(DEFINED ANDROID_PLATFORM) # native_app_glue for Android
  add_library(native_app_glue STATIC
    ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
  target_include_directories(native_app_glue PUBLIC
    ${ANDROID_NDK}/sources/android/native_app_glue)
  target_include_directories(small3d PUBLIC
    ${ANDROID_NDK}/sources/android/native_app_glue)
endif()

if(NOT DEFINED ANDROID_PLATFORM AND NOT IOS) # PC, OpenGL
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

# on windows bzip2 is required for gcc version >= 10, otherwise a freetype associated
# link error is produced
if(UNIX OR (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" AND NOT
      CMAKE_CXX_COMPILER_VERSION VERSION_LESS 10))
  target_include_directories(small3d PUBLIC ${BZIP2_INCLUDE_DIRS})
  target_link_libraries(small3d PUBLIC ${BZIP2_LIBRARIES})
endif()

if(WIN32) # Windows
  target_link_libraries(small3d PUBLIC winmm)
endif()

if(UNIX AND NOT APPLE) # FreeBSD or Linux
  message(STATUS "System: ${CMAKE_SYSTEM_NAME}")
  if(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    link_directories(/usr/local/lib)
    target_link_libraries(small3d PUBLIC m pthread rt jack X11 dl)
  else() 
    target_link_libraries(small3d PUBLIC m pthread rt asound X11 dl)
  endif()
endif()

if(NOT DEFINED ANDROID_PLATFORM AND NOT IOS) # PC

  if(APPLE) # MacOS
    target_link_libraries(small3d PUBLIC "-framework \
AudioUnit -framework AudioToolbox -framework CoreAudio -framework Cocoa \
-framework IOKit -framework CoreVideo")
  endif()

  subdirs(UnitTests FormatConverter)

endif()
