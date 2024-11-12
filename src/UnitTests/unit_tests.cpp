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
      return EXIT_FAILURE;
    }

    small3d::initLogger();

    LOGINFO("LoggerTest OK");

    if (!MathTest()) {
      LOGINFO("*** Failing MathTest.");
      return EXIT_FAILURE;
    }
    LOGINFO("MathTest OK");
    
#ifdef _WIN32
    if (!ScreenCaptureTest()) {
      LOGINFO("*** Failing ScreenCaptureTest.");
      return EXIT_FAILURE;
    }
    LOGINFO("ScreenCaptureTest OK");

    if (!ControllerTest()) {
      LOGINFO("*** Failing ControllerTest.");
      return EXIT_FAILURE;
    }
    LOGINFO("ControllerTest OK");
#endif

    if (!ImageTest()) {
      LOGINFO("*** Failing ImageTest.");
      return EXIT_FAILURE;
    }
    LOGINFO("ImageTest OK");

    if (!WavefrontFailTest()) {
      LOGINFO("*** Failing WavefrontFailTest.");
      return EXIT_FAILURE;
    }
    LOGINFO("WavefrontFailTest OK");
    
    if (!WavefrontModelTest()) {
      LOGINFO("*** Failing WavefrontModelTest.");
      return EXIT_FAILURE;
    }
    LOGINFO("WavefrontModelTest OK");

    if (!ScaleAndTransformTest()) {
      LOGINFO("*** Failing ScaleAndTransformTest.");
      return EXIT_FAILURE;
    }
    LOGINFO("ScaleAndTransformTest OK");
    
    if (!GlbTextureTestDefaultShadows()) {
      LOGINFO("*** Failing GlbTextureTestDefaultShadows.");
      return EXIT_FAILURE;
    }
    LOGINFO("GlbTextureTestDefaultShadows OK");
    
    if (!BoundingBoxesTest()) {
      LOGINFO("*** Failing BoundingBoxesTest.");
      return EXIT_FAILURE;
    }
    LOGINFO("BoundingBoxesTest OK");
    
    if (!FPStest()) {
      LOGINFO("*** Failing FPStest.");
      return EXIT_FAILURE;
    }
    LOGINFO("FPStest OK");

    if (!GenericSceneObjectConstructorTest()) {
      LOGINFO("*** Failing GenericSceneObjectConstructorTest.");
      return EXIT_FAILURE;
    }
    LOGINFO("GenericSceneObjectConstructorTest OK");
    
    if (!RendererTest()) {
      LOGINFO("*** Failing RendererTest.");
      return EXIT_FAILURE;
    }
    LOGINFO("RendererTest OK");

    if (!BinaryModelTest()) {
      LOGINFO("*** Failing BinaryModelTest.");
      return EXIT_FAILURE;
    }
    LOGINFO("BinaryModelTest OK");

    if (!SoundTest()) {
      LOGINFO("*** Failing SoundTest.");
      return EXIT_FAILURE;
    }
    LOGINFO("SoundTest OK");

    if (!BinSoundTest()) {
      LOGINFO("*** Failing BinSoundTest.");
      return EXIT_FAILURE;
    }
    LOGINFO("BinSoundTest OK");

    if (!SoundTest2()) {
      LOGINFO("*** Failing SoundTest2.");
      return EXIT_FAILURE;
    }
    LOGINFO("SoundTest2 OK");

    if (!SoundTest3()) {
      LOGINFO("*** Failing SoundTest3.");
      return EXIT_FAILURE;
    }
    LOGINFO("SoundTest3 OK");
    
    if (!GlbTest()) {
      LOGINFO("*** Failing GlbTest.");
      return EXIT_FAILURE;
    }
    LOGINFO("GlbTest OK");

    if (!ModelsTimeToLoad()) {
      LOGINFO("*** Failing ModelsTimeToLoad.");
      return EXIT_FAILURE;
    }
    LOGINFO("ModelsTimeToLoad OK");
    

    LOGINFO("All tests have executed successfully.\n\r");
  }
  catch (std::exception& e) {
    // Also use printf in case the logger has not been initialised properly
    printf("*** Exception thrown during unit tests: %s\n\r", e.what());
    LOGERROR("*** Exception thrown during unit tests: " + std::string(e.what()));
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
