add_library(small3d BasePath.cpp BoundingBoxSet.cpp File.cpp GlbFile.cpp
  WavefrontFile.cpp BinaryFile.cpp Image.cpp Logger.cpp Model.cpp Renderer.cpp
  SceneObject.cpp  Sound.cpp Time.cpp Material.cpp Math.cpp Windowing.cpp
  ../include/small3d/SceneObject.hpp
  ../include/small3d/Sound.hpp ../include/small3d/Time.hpp
  ../include/small3d/BasePath.hpp ../include/small3d/BoundingBoxSet.hpp
  ../include/small3d/File.hpp ../include/small3d/GlbFile.hpp
  ../include/small3d/WavefrontFile.hpp ../include/small3d/BinaryFile.hpp
  ../include/small3d/Image.hpp ../include/small3d/Logger.hpp
  ../include/small3d/Model.hpp ../include/small3d/Renderer.hpp
  ../include/small3d/Material.hpp
  ../include/small3d/Math.hpp
  ../include/small3d/Windowing.hpp
)

target_include_directories(small3d PUBLIC ../include/small3d)

target_include_directories(small3d PUBLIC
  ${PNG_INCLUDE_DIRS}
  ${OGG_INCLUDE_DIRS}
  ${VORBIS_INCLUDE_DIR}
  ${FREETYPE_INCLUDE_DIRS}
  ${GLFW_INCLUDE_DIRS}
  ${PORTAUDIO_INCLUDE_DIRS}
)

if(WIN32)
  target_compile_definitions(small3d PRIVATE GLEW_STATIC)
endif()

target_compile_definitions(small3d PRIVATE ${PNG_DEFINITIONS})


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

if(UNIX AND NOT APPLE) # Linux
  message(STATUS "System: ${CMAKE_SYSTEM_NAME}")
  target_link_libraries(small3d PUBLIC m pthread rt asound X11 dl)
endif()

if(APPLE) # MacOS
  target_link_libraries(small3d PUBLIC "-framework \
AudioUnit -framework AudioToolbox -framework CoreAudio -framework Cocoa \
-framework IOKit -framework CoreVideo")
endif()

