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

int main(int argc, char** argv) {

  try
  { 
    if (!LoggerTest()) {
      printf("*** Failing LoggerTest.\n\r");
      return 1;
    }

    small3d::initLogger();

    LOGINFO("LoggerTest OK");

#ifdef _WIN32
    if (!ScreenCaptureTest()) {
      LOGINFO("*** Failing ScreenCaptureTest.");
      return 1;
    }
    LOGINFO("ScreenCaptureTest OK");

    if (!ControllerTest()) {
      LOGINFO("*** Failing ControllerTest.");
      return 1;
    }
    LOGINFO("ControllerTest OK");
#endif

    if (!ImageTest()) {
      LOGINFO("*** Failing ImageTest.");
      return 1;
    }
    LOGINFO("ImageTest OK");

    if (!WavefrontFailTest()) {
      LOGINFO("*** Failing WavefrontFailTest.");
      return 1;
    }
    LOGINFO("WavefrontFailTest OK");
    
    if (!WavefrontModelTest()) {
      LOGINFO("*** Failing WavefrontModelTest.");
      return 1;
    }
    LOGINFO("WavefrontModelTest OK");

    if (!ScaleAndTransformTest()) {
      LOGINFO("*** Failing ScaleAndTransformTest.");
      return 1;
    }
    LOGINFO("ScaleAndTransformTest OK");
    
    if (!GlbTextureTestDefaultShadows()) {
      LOGINFO("*** Failing GlbTextureTestDefaultShadows.");
      return 1;
    }
    LOGINFO("GlbTextureTestDefaultShadows OK");
    
    if (!GlbTextureTestLookAtShadows()) {
      LOGINFO("*** Failing GlbTextureTestLookAtShadows.");
      return 1;
    }
    LOGINFO("GlbTextureTestLookAtShadows OK");
    
    if (!BoundingBoxesTest()) {
      LOGINFO("*** Failing BoundingBoxesTest.");
      return 1;
    }
    LOGINFO("BoundingBoxesTest OK");
    
    if (!FPStest()) {
      LOGINFO("*** Failing FPStest.");
      return 1;
    }
    LOGINFO("FPStest OK");

    if (!GenericSceneObjectConstructorTest()) {
      LOGINFO("*** Failing GenericSceneObjectConstructorTest.");
      return 1;
    }
    LOGINFO("GenericSceneObjectConstructorTest OK");
    
    if (!RendererTest()) {
      LOGINFO("*** Failing RendererTest.");
      return 1;
    }
    LOGINFO("RendererTest OK");

    if (!BinaryModelTest()) {
      LOGINFO("*** Failing BinaryModelTest.");
      return 1;
    }
    LOGINFO("BinaryModelTest OK");

    if (!SoundTest()) {
      LOGINFO("*** Failing SoundTest.");
      return 1;
    }
    LOGINFO("SoundTest OK");

    if (!BinSoundTest()) {
      LOGINFO("*** Failing BinSoundTest.");
      return 1;
    }
    LOGINFO("BinSoundTest OK");

    if (!SoundTest2()) {
      LOGINFO("*** Failing SoundTest2.");
      return 1;
    }
    LOGINFO("SoundTest2 OK");

    if (!SoundTest3()) {
      LOGINFO("*** Failing SoundTest3.");
      return 1;
    }
    LOGINFO("SoundTest3 OK");
    
    if (!GlbTest()) {
      LOGINFO("*** Failing GlbTest.");
      return 1;
    }
    LOGINFO("GlbTest OK");

    if (!ModelsTimeToLoad()) {
      LOGINFO("*** Failing ModelsTimeToLoad.");
      return 1;
    }
    LOGINFO("ModelsTimeToLoad OK");

    LOGINFO("All tests have executed successfully.\n\r");
  }
  catch (std::exception& e) {
    // Also use printf in case the logger has not been initialised properly
    printf("*** Exception thrown during unit tests: %s\n\r", e.what());
    LOGERROR("*** Exception thrown during unit tests: " + std::string(e.what()));
    return 1;
  }

  return 0;
}
