/*
 *  unit_tests.cpp
 *
 *  Created on: 2014/10/18
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */
#include <iostream>
#include <small3d/Renderer.hpp>
#include <small3d/Time.hpp>
#include <small3d/Logger.hpp>
#include <small3d/Image.hpp>
#include <small3d/Model.hpp>
#include <small3d/SceneObject.hpp>
#include <small3d/Sound.hpp>
#include <small3d/BoundingBoxSet.hpp>
#include <small3d/GlbFile.hpp>
#include <small3d/WavefrontFile.hpp>
#include "OctPyramid.hpp"
#ifndef SMALL3D_OPENGL
#include "vulkan_helper.h"
#endif

using namespace small3d;
using namespace std;

Renderer *r = nullptr;

#if defined(__ANDROID__)
#define RETURN1
#define RETURN0

bool appActive = false;
bool instantiated = false;
uint32_t screenWidth, screenHeight;
#else
#define RETURN1 return 1;
#define RETURN0 return 0;
#endif

#if !defined(__ANDROID__) && !defined(SMALL3D_IOS)
void pollEvents() {
  glfwPollEvents();
}
#else
void pollEvents() {

}
#endif

void initRenderer() {
#if defined(__ANDROID__) || defined(SMALL3D_IOS)
  if (r == nullptr) {
    r = &small3d::Renderer::getInstance("Islet Hell", 854, 480, 0.785f, 1.0f, 24.0f,
                                        "resources/shaders/", 5000);
  }
#else
  #if !defined(NDEBUG) && defined(WIN32)
  r = &small3d::Renderer::getInstance("Islet Hell", 1024, 768);
#else
  r = &small3d::Renderer::getInstance("Islet Hell");
#endif
#endif
}

#if defined(__ANDROID__)

void get_screen_info() {
  screenWidth = r->getScreenWidth();
  screenHeight = r->getScreenHeight();
}

void handle_cmd(android_app *pApp, int32_t cmd) {
  switch (cmd) {
    case APP_CMD_INIT_WINDOW:
    case APP_CMD_GAINED_FOCUS:
      if (!appActive) {
        if (!instantiated) {
          initRenderer();
          instantiated = true;

        } else {
          r->setupVulkan();

        }

        get_screen_info();
        appActive = true;
      }
      break;

    case APP_CMD_TERM_WINDOW:
    case APP_CMD_LOST_FOCUS:
    case APP_CMD_SAVE_STATE:
    case APP_CMD_STOP:
      if (appActive) {
        r->destroyVulkan();
        appActive = false;
      }
      break;

    default:
      LOGDEBUG("event not handled: " + std::to_string(cmd));
  }
}

#endif

int LoggerTest() {
  deleteLogger();
  ostringstream oss;
  initLogger();
  LOGINFO("Logger info test works");
  LOGERROR("Logger error test works");
  deleteLogger();
  return 1;
}

int ImageTest() {
  
  Image image("resources/images/testImage.png");

  cout << "Image width " << image.getWidth() << ", height " <<
    image.getHeight() << endl;

  const float* imageData = image.getData();

  unsigned long x = 0, y = 0;

  while (y < image.getHeight()) {
    x = 0;
    while (x < image.getWidth()) {
      
      const float *colour = &imageData[4 * y * image.getWidth() + 4 * x];       
      
      if (colour[0] <  0.0f) return 0;
      if (colour[0] > 1.0f) return 0;
      if (colour[1] < 0.0f) return 0;
      if (colour[1] > 1.0f) return 0;
      if (colour[2] < 0.0f) return 0;
      if (colour[2] > 1.0f) return 0;
      if (colour[3] != 1.0f) return 0;
      
      ++x;
    }
    ++y;
  }
  return 1;
}

int WavefrontTest() {

  WavefrontFile wf("resources/models/goat.glb");
  bool threw = false;
  try {
    Model m;
    wf.load(m, "");
  }
  catch (std::runtime_error& e) {
    LOGINFO("WavefrontFile.load correctly threw a runtime error: " +
      std::string(e.what()));
    threw = true;
  }

  if (!threw) throw std::runtime_error("WavefrontFile.load has not thrown"
    " a runtime error, as it should have.");

  return 1;
}

int WavefrontModelTest() {
  Model model(WavefrontFile("resources/models/Cube/Cube.obj"), "");

  if (model.vertexData.size() == 0) return 0;
  if (model.indexData.size() == 0) return 0;
  if (model.normalsData.size() == 0) return 0;
  if (model.textureCoordsData.size() == 0) return 0;
  
  cout << "Vertex data component count: "
    << model.vertexData.size() << endl << "Index count: "
    << model.indexData.size() << endl
    << "Normals data component count: "
    << model.normalsData.size() << endl
    << "Texture coordinates count: "
    << model.textureCoordsData.size() << endl;

  Model modelWithNoTexture(WavefrontFile("resources/models/Cube/CubeNoTexture.obj"), "");

  if (modelWithNoTexture.vertexData.size() == 0) return 0;
  if (modelWithNoTexture.indexData.size() == 0) return 0;
  if (modelWithNoTexture.normalsData.size() == 0) return 0;

  cout << "Vertex data component count: "
    << modelWithNoTexture.vertexData.size() << endl << "Index count: "
    << modelWithNoTexture.indexData.size() << endl
    << "Normals data component count: "
    << modelWithNoTexture.normalsData.size() << endl
    << "Texture coordinates count: "
    << modelWithNoTexture.textureCoordsData.size() << endl;

  WavefrontFile w2("resources/models/goatAndTree.obj");

  Model model2(w2, "Cube.001");
  Model model3(w2, "Cube");
  Model model4(w2, "");

  initRenderer();

  double startSeconds = getTimeInSeconds();
  double seconds = getTimeInSeconds();
  double prevSeconds = seconds;
  const uint32_t framerate = 30;

  constexpr double secondsInterval = 1.0 / framerate;

  model3.scale += 0.3f;

  while (seconds - startSeconds < 5.0) {
    pollEvents();
    seconds = getTimeInSeconds();
    if (seconds - prevSeconds > secondsInterval) {
      prevSeconds = seconds;
      
      r->render(model2, glm::vec3(-1.5f, -1.0f, -3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
      r->render(model3, glm::vec3(0.0f, -1.0f, -2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
      r->render(model4, glm::vec3(1.5f, -1.0f, -3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));

      r->swapBuffers();
    }
  }

  return 1;
}

int ScaleAndTransformTest() {
  initRenderer();

  double startSeconds = getTimeInSeconds();
  double seconds = getTimeInSeconds();
  double prevSeconds = seconds;
  const uint32_t framerate = 30;
  
  constexpr double secondsInterval = 1.0 / framerate;

  SceneObject boxes("boxes", "resources/models/boxes.glb", "");

  boxes.position = glm::vec3(0.0f, 0.0f, -3.0f);
  
  while (seconds - startSeconds < 5.0) {
    pollEvents();
    seconds = getTimeInSeconds();
    if (seconds - prevSeconds > secondsInterval) {
      prevSeconds = seconds;

      r->render(boxes, glm::vec4(0.5f, 0.3f, 0.0f, 1.0f));

      r->swapBuffers();
      boxes.rotate(glm::vec3(0.0f, 0.01f, 0.0f));
    }
  }

  return 1;
}

int GlbTextureTest() {
  initRenderer();

  double startSeconds = getTimeInSeconds();
  double seconds = getTimeInSeconds();
  double prevSeconds = seconds;
  const uint32_t framerate = 30;

  constexpr double secondsInterval = 1.0 / framerate;

  SceneObject goat("goat5", "resources/models/goatAndTree.glb", "Cube");

  r->generateTexture("goatGlbTexture", *goat.getModel().defaultTextureImage);

  SceneObject tree("tree5", "resources/models/goatAndTree.glb", "Cube.001");

  r->generateTexture("treeGlbTexture", *tree.getModel().defaultTextureImage);

  goat.position = glm::vec3(0.0f, 0.0f, -3.0f);
  goat.startAnimating();
  tree.position = glm::vec3(0.0f, 0.0f, -4.0f);

  while (seconds - startSeconds < 5.0) {
    pollEvents();
    seconds = getTimeInSeconds();
    if (seconds - prevSeconds > secondsInterval) {
      prevSeconds = seconds;
      goat.animate();

      r->render(goat, "goatGlbTexture");
      r->render(tree, "treeGlbTexture");

      r->swapBuffers();
      goat.rotate(glm::vec3(0.0f, 0.01f, 0.0f));
    }
  }

  return 1;
}

int BoundingBoxesTest() {

  initRenderer();

  double startSeconds = getTimeInSeconds();
  double seconds = getTimeInSeconds();
  double prevSeconds = seconds;
  const uint32_t framerate = 30;

  constexpr double secondsInterval = 1.0 / framerate;

  SceneObject goat("goat", "resources/models/goatUnscaled.glb", "Cube", 3);
  auto boundingBoxModels = goat.getBoundingBoxSetModels();
  goat.position = glm::vec3(0.0f, 0.0f, -3.0f);
  goat.startAnimating();

  while (seconds - startSeconds < 5.0) {
    pollEvents();
    seconds = getTimeInSeconds();
    if (seconds - prevSeconds > secondsInterval) {
      prevSeconds = seconds;

      goat.animate();

      r->render(goat, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

      for (auto& m : boundingBoxModels) {
        r->render(m, goat.position,
          goat.getTransformation(), glm::vec4(5.0f, 5.0f, 1.0f, 0.5f));
      }
      r->swapBuffers();
      goat.rotate(glm::vec3(0.0f, 0.01f, 0.0f));
    }
  }

  SceneObject goat2("goat2", goat.getModel());
  if (goat2.getBoundingBoxSetExtremes().size() == 0) {
    LOGERROR("Bounding boxes not created for goat2");
    return 0;
  }

  SceneObject goat3("goat3", goat.getModel(), 2);
  if (goat3.getBoundingBoxSetExtremes().size() == 0) {
    LOGERROR("Bounding boxes not created for goat3");
    return 0;
  }

  OctPyramid op(4.4f, 7.5f);
  SceneObject opobj("op", op);
  if (opobj.getBoundingBoxSetExtremes().size() == 0) {
    LOGERROR("Bounding boxes not created for op");
    return 0;
  }

  SceneObject op1obj("op1", op, 2);
  if (op1obj.getBoundingBoxSetExtremes().size() == 0) {
    LOGERROR("Bounding boxes not created for op1");
    return 0;
  }

  return 1;
}

int FPStest() {

  initRenderer();

  double startSeconds = getTimeInSeconds();
  double seconds = getTimeInSeconds();
  double prevSeconds = seconds;
  uint32_t framerate = 0;
  uint32_t numFrames = 0;

  SceneObject goat("goat", "resources/models/goatUnscaled.glb", "Cube", 3);
  auto boundingBoxModels = goat.getBoundingBoxSetModels();
  goat.position = glm::vec3(0.0f, 0.0f, -3.0f);
  goat.startAnimating();

  Model texturedRect;

  r->createRectangle(texturedRect, glm::vec3(0.0f, 0.5f, 0.0f),
    glm::vec3(1.0f, -1.0f, 0.0f));


  while (seconds - startSeconds < 10.0) {
    pollEvents();
    seconds = getTimeInSeconds();
    if (seconds - prevSeconds > 1.0) {
      framerate = numFrames;
      numFrames = 0;
      prevSeconds = seconds;
    }

      goat.animate(); // on android shader will ignore animation
                      // (it cannot be loaded on Adreno with joints logic - shader compilation error
                      // is returned)

      r->render(goat, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

      for (auto& m : boundingBoxModels) {
        r->render(m, goat.position,
          goat.getTransformation(), glm::vec4(5.0f, 5.0f, 1.0f, 0.5f));
      }

      r->generateTexture("frameRate", std::to_string(framerate) + " FPS",
        glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));

      r->render(texturedRect,
        glm::vec3(0.0f, 0.0f, -1.0f),
        glm::vec3(0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), "frameRate", false);

      r->swapBuffers();
      ++numFrames;
      goat.rotate(glm::vec3(0.0f, 0.01f, 0.0f));
    
  }

  return 1;
}

int RendererTest() {
  initRenderer();

  r->setCameraRotation(glm::vec3(0.4f, 0.1f, 0.1f));
  
  // Here loading the mesh without providing a name is also tested.
  Model modelFromGlb(GlbFile("resources/models/goatUnscaled.glb"), ""); 

  SceneObject object("cube", "resources/models/Cube/CubeNoTexture.obj");
  object.position = glm::vec3(0.0f, -1.0f, -8.0f);
  r->render(object, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

  SceneObject object2("texutredCube", "resources/models/Cube/Cube.obj");
  object2.position = glm::vec3(-2.0f, -1.0f, -7.0f);
  object2.setRotation(glm::vec3(0.3f, 1.3f, 0.0f));

  Image cubeTexture("resources/models/Cube/cubeTexture.png");
  r->generateTexture("cubeTexture", cubeTexture);
#if !defined(__ANDROID__) && !defined(SMALL3D_IOS)
  glfwShowWindow(r->getWindow());
#endif
  Model singleColourRect;
  r->createRectangle(singleColourRect, glm::vec3(-1.0f, 0.0f, 0.0f),
    glm::vec3(-0.5f, -0.5f, 0.0f));

  Model texturedRect;

  r->createRectangle(texturedRect, glm::vec3(0.0f, 0.5f, 0.0f),
    glm::vec3(1.0f, -1.0f, 0.0f));

  r->generateTexture("small3dTexture", "small3d :)",
    glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));

  Model textRect;

  r->createRectangle(textRect, glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.5f, -0.5f, 0.0f));

  double startSeconds = getTimeInSeconds();
  double seconds = getTimeInSeconds();
  double prevSeconds = seconds;
  const uint32_t framerate = 30;

  constexpr double secondsInterval = 1.0 / framerate;

  glm::vec3 rotation(0.0f, 0.0f, 0.0f);

  while (seconds - startSeconds < 5.0) {
    pollEvents();
    seconds = getTimeInSeconds();
    if (seconds - prevSeconds > secondsInterval) {
      prevSeconds = seconds;

      r->render(singleColourRect,
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), "", false);

      r->generateTexture("small3dTexture", std::to_string(modelFromGlb.getCurrentPoseIdx()),
        glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));

      r->render(texturedRect,
        glm::vec3(0.0f, 0.0f, -2.0f),
        glm::vec3(0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), "cubeTexture", true);

      r->render(object2, "cubeTexture");

      modelFromGlb.animate();
      rotation.y += 0.1f;

      r->render(modelFromGlb, glm::vec3(0.0f, 1.0f, -6.0f),
        rotation, glm::vec4(0.3f, 1.0f, 1.0f, 1.0f));

      r->render(textRect, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), "small3dTexture", false);

      r->swapBuffers();
      
    }
  }
  r->clearBuffers(object);
  r->clearBuffers(object2);
  r->clearBuffers(texturedRect);
  r->clearBuffers(textRect);
  r->deleteTexture("cubeTexture");

  return 1;
}

int SoundTest() {
  Sound snd("resources/sounds/bah.ogg");
  snd.play();
  double startSeconds = getTimeInSeconds();
  while (getTimeInSeconds() - startSeconds < 0.5);
  snd.stop();

  snd.divideVolume(4);
  snd.play();
  startSeconds = getTimeInSeconds();
  while (getTimeInSeconds() - startSeconds < 0.5);
  snd.stop();

  startSeconds = getTimeInSeconds();
  while (getTimeInSeconds() - startSeconds < 0.5);
  snd.play();
  // Make sure the sound is stopped by the stop function and not the destructor.
  startSeconds = getTimeInSeconds();
  while(getTimeInSeconds() - startSeconds < 2.0);
  return 1;
}

int SoundTest2() {
  Sound snd1("resources/sounds/bah.ogg");
  Sound snd2(snd1);
  Sound snd3 = snd2;
  snd1.play();
  double startSeconds = getTimeInSeconds();
  while (getTimeInSeconds() - startSeconds < 0.3);
  snd2.play();
  startSeconds = getTimeInSeconds();
  while (getTimeInSeconds() - startSeconds < 0.3);
  snd3.play();
  startSeconds = getTimeInSeconds();
  while(getTimeInSeconds() - startSeconds < 1.0);
  return 1;
}

int SoundTest3() {
  Sound snd("resources/sounds/bah.ogg");
  snd.play(true);
  double startSeconds = getTimeInSeconds();
  while(getTimeInSeconds() - startSeconds < 6.0);
  return 1;
}

int GlbTest() {

  GlbFile glb("resources/models/goat.glb");

  glb.printTokensRecursive();

  glb.printTokensSerial();

  return 1;
}

int GenericSceneObjectConstructorTest() {

  SceneObject so1("goat1", "resources/models/goat.glb", "");
  SceneObject so2("goat2", "resources/models/goat.obj", "");

  if (so1.getModel().vertexDataByteSize == 0) return 0;
  if (so2.getModel().vertexDataByteSize == 0) return 0;

  return 1;
}
#if defined(__ANDROID__)
extern "C" {
void android_main(struct android_app* state) {
  vh_android_app = state;
  vh_android_app->onAppCmd = handle_cmd;


  const uint32_t frameRate = 60;
  double seconds = getTimeInSeconds();
  double prevSeconds = seconds;
  double secondsInterval = 1.0 / frameRate;
  int events;
  android_poll_source *pSource;
  do {

    if (ALooper_pollAll(0, nullptr, &events, (void **) &pSource) >= 0) {
      if (pSource != NULL) {
        pSource->process(state, pSource);
      }
    }

    if (appActive) {
      seconds = getTimeInSeconds();

      if (seconds - prevSeconds > secondsInterval) {
        prevSeconds = seconds;
        FPStest();
        break;
      }

    }

  } while (!state->destroyRequested);
  std::terminate();
return;
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
      printf("*** Failing WavefrontTest.\n\r");
      RETURN1
    }

    if (!WavefrontModelTest()) {
      printf("*** Failing WavefrontModelTest.\n\r");
      RETURN1
    }

    if (!ScaleAndTransformTest()) {
      printf("*** Failing GlbTextureText.\n\r");
      RETURN1
    }

    if (!GlbTextureTest()) {
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
  catch (exception& e) {
    printf("*** %s\n\r", e.what());
    RETURN1
  }
  printf("All tests have executed successfully.\n\r");
  RETURN0
}
#if defined(__ANDROID__)
}
#endif
