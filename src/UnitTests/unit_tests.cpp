/*
 *  unit_tests.cpp
 *
 *  Created on: 2014/10/18
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#include "UnitTests.hpp"
#include <iostream>
#include <Time.hpp>

#if defined(__ANDROID__)
#define RETURN1
#define RETURN0
#else
#define RETURN1 return 1;
#define RETURN0 return 0;
#endif

int main(int argc, char** argv) {

  try
  { 
    if (!LoggerTest()) {
      printf("*** Failing LoggerTest.\n\r");
      RETURN1
    }

    small3d::initLogger();

    LOGINFO("LoggerTest OK");

#ifdef _WIN32
    if (!ScreenCaptureTest()) {
      LOGINFO("*** Failing ScreenCaptureTest.");
      RETURN1
    }
    LOGINFO("ScreenCaptureTest OK");

    if (!ControllerTest()) {
      LOGINFO("*** Failing ControllerTest.");
      RETURN1
    }
    LOGINFO("ControllerTest OK");
#endif

    if (!ImageTest()) {
      LOGINFO("*** Failing ImageTest.");
      RETURN1
    }
    LOGINFO("ImageTest OK");

    if (!WavefrontFailTest()) {
      LOGINFO("*** Failing WavefrontFailTest.");
      RETURN1
    }
    LOGINFO("WavefrontFailTest OK");
    
    if (!WavefrontModelTest()) {
      LOGINFO("*** Failing WavefrontModelTest.");
      RETURN1
    }
    LOGINFO("WavefrontModelTest OK");

    if (!ScaleAndTransformTest()) {
      LOGINFO("*** Failing ScaleAndTransformTest.");
      RETURN1
    }
    LOGINFO("ScaleAndTransformTest OK");
    
    if (!GlbTextureTestDefaultShadows()) {
      LOGINFO("*** Failing GlbTextureTestDefaultShadows.");
      RETURN1
    }
    LOGINFO("GlbTextureTestDefaultShadows OK");
    
    if (!GlbTextureTestLookAtShadows()) {
      LOGINFO("*** Failing GlbTextureTestLookAtShadows.");
      RETURN1
    }
    LOGINFO("GlbTextureTestLookAtShadows OK");
    
    if (!BoundingBoxesTest()) {
      LOGINFO("*** Failing BoundingBoxesTest.");
      RETURN1
    }
    LOGINFO("BoundingBoxesTest OK");
    
    if (!FPStest()) {
      LOGINFO("*** Failing FPStest.");
      RETURN1
    }
    LOGINFO("FPStest OK");

    if (!GenericSceneObjectConstructorTest()) {
      LOGINFO("*** Failing GenericSceneObjectConstructorTest.");
      RETURN1
    }
    LOGINFO("GenericSceneObjectConstructorTest OK");
    
    if (!RendererTest()) {
      LOGINFO("*** Failing RendererTest.");
      RETURN1
    }
    LOGINFO("RendererTest OK");

    if (!BinaryModelTest()) {
      LOGINFO("*** Failing BinaryModelTest.");
      RETURN1
    }
    LOGINFO("BinaryModelTest OK");

    if (!SoundTest()) {
      LOGINFO("*** Failing SoundTest.");
      RETURN1
    }
    LOGINFO("SoundTest OK");

    if (!BinSoundTest()) {
      LOGINFO("*** Failing BinSoundTest.");
      RETURN1
    }
    LOGINFO("BinSoundTest OK");

    if (!SoundTest2()) {
      LOGINFO("*** Failing SoundTest2.");
      RETURN1
    }
    LOGINFO("SoundTest2 OK");

    if (!SoundTest3()) {
      LOGINFO("*** Failing SoundTest3.");
      RETURN1
    }
    LOGINFO("SoundTest3 OK");
    
    if (!GlbTest()) {
      LOGINFO("*** Failing GlbTest.");
      RETURN1
    }
    LOGINFO("GlbTest OK");

    if (!ModelsTimeToLoad()) {
      LOGINFO("*** Failing ModelsTimeToLoad.");
      RETURN1
    }
    LOGINFO("ModelsTimeToLoad OK");

    LOGINFO("All tests have executed successfully.\n\r");
  }
  catch (std::exception& e) {
    // Also use printf in case the logger has not been initialised properly
    printf("*** Exception thrown during unit tests: %s\n\r", e.what());
    LOGERROR("*** Exception thrown during unit tests: " + std::string(e.what()));
    RETURN1
  }

  RETURN0
}

