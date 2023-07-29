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

#if defined(__ANDROID__)
extern "C" {
void android_main(struct android_app* state) {

  small3d_android_app = state;
  
  small3d_android_app->onAppCmd = handle_cmd;
  while(!appActive) {
    pollEvents();
  }
#else
int main(int argc, char** argv) {
#endif
  try
  { 
    if (!LoggerTest()) {
      printf("*** Failing LoggerTest.\n\r");
      RETURN1
    }

    small3d::initLogger();
    LOGINFO("LoggerTest OK");

    if (!ImageTest()) {
      LOGINFO("*** Failing ImageTest.");
      RETURN1
    }
    LOGINFO("ImageTest OK");

    if (!WavefrontTest()) {
      LOGINFO("*** Failing ImageTest.");
      RETURN1
    }
    LOGINFO("WavefrontTest OK");

    if (!WavefrontModelTest()) {
      LOGINFO("*** Failing ImageTest.");
      RETURN1
    }
    LOGINFO("WavefrontModelTest OK");
    
    if (!ScaleAndTransformTest()) {
      LOGINFO("*** Failing GlbTextureText.");
      RETURN1
    }
    LOGINFO("ScaleAndTransformTest OK");
    
    if (!GlbTextureTestDefaultShadows()) {
      LOGINFO("*** Failing GlbTextureText.");
      RETURN1
    }
    LOGINFO("GlbTextureTestDefaultShadows OK");
    
    if (!GlbTextureTestLookAtShadows()) {
      LOGINFO("*** Failing GlbTextureText.");
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

    if (!ModelsTimeToLoad()) {
      LOGINFO("*** Failing GlbTest.");
      RETURN1
    }

    LOGINFO("GlbTest OK");
  }
  catch (std::exception& e) {
    printf("*** %s\n\r", e.what());
    RETURN1
  }
  LOGINFO("All tests have executed successfully.\n\r");

#ifdef __ANDROID__
  while(appActive) {
    pollEvents();
  }
#endif
  RETURN0
}
#if defined(__ANDROID__)
} // end extern "C"
#endif
