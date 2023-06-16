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
#define RETURN1 std::terminate();
#define RETURN0 std::terminate();
#else
#define RETURN1 return 1;
#define RETURN0 return 0;
#endif

#if defined(__ANDROID__)
extern "C" {
void android_main(struct android_app* state) {

  small3d_android_app = state;
#ifndef SMALL3D_OPENGLES
  vh_android_app = state;
#endif
  
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

    if (!ImageTest()) {
      printf("*** Failing ImageTest.\n\r");
      RETURN1
    }

    if (!WavefrontTest()) {
      printf("*** Failing ImageTest.\n\r");
      RETURN1
    }

    if (!WavefrontModelTest()) {
      printf("*** Failing ImageTest.\n\r");
      RETURN1
    }
    
    if (!ScaleAndTransformTest()) {
      printf("*** Failing GlbTextureText.\n\r");
      RETURN1
    }
    
    if (!GlbTextureTestDefaultShadows()) {
      printf("*** Failing GlbTextureText.\n\r");
      RETURN1
    }
    
    if (!GlbTextureTestLookAtShadows()) {
      printf("*** Failing GlbTextureText.\n\r");
      RETURN1
    }
    
    if (!BoundingBoxesTest()) {
      printf("*** Failing BoundingBoxesTest.\n\r");
      RETURN1
    }
    
    if (!FPStest()) {
      printf("*** Failing FPStest.\n\r");
      RETURN1
    }

    if (!GenericSceneObjectConstructorTest()) {
      printf("*** Failing GenericSceneObjectConstructorTest.\n\r");
      RETURN1
    } 
    
    if (!RendererTest()) {
      printf("*** Failing RendererTest.\n\r");
      RETURN1
    }
    
    if (!SoundTest()) {
      printf("*** Failing SoundTest.\n\r");
      RETURN1
    }

    if (!SoundTest2()) {
      printf("*** Failing SoundTest2.\n\r");
      RETURN1
    }

    if (!SoundTest3()) {
      printf("*** Failing SoundTest3.\n\r");
      RETURN1
    }

    if (!GlbTest()) {
      printf("*** Failing GlbTest.\n\r");
      RETURN1
    } 
    
  }
  catch (std::exception& e) {
    printf("*** %s\n\r", e.what());
    RETURN1
  }
  printf("All tests have executed successfully.\n\r");
  RETURN0
}
#if defined(__ANDROID__)
}
#endif
