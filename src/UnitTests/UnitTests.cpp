/*
 *  UnitTests.cpp
 *
 *  Created on: 2022/09/26
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#include "UnitTests.hpp"
#include <iostream>
#include "Time.hpp"
#include "Logger.hpp"
#include "Image.hpp"
#include "Model.hpp"
#include "SceneObject.hpp"
#include "Sound.hpp"
#include "BoundingBoxSet.hpp"
#include "GlbFile.hpp"
#include "WavefrontFile.hpp"
#include "BinaryFile.hpp"
#include <thread>

using namespace small3d;
using namespace std;

Renderer* r = nullptr;

std::string resourceDir = "resources";


void pollEvents() {
  glfwPollEvents();
}

static small3d::Model indicator;

void write(const std::string& text, float elevation) {
  if (indicator.vertexData.empty()) {
    r->createRectangle(indicator, glm::vec3(-0.4f, -0.4f, 0.1f),
      glm::vec3(0.4f, -0.50f, 0.1f));
  }
  std::string textureName = "indication" + std::to_string(elevation);
  r->generateTexture(textureName, text, glm::vec3(1.0f, 1.0f, 1.0f), 24);
  r->render(indicator, glm::vec3(0.0f, elevation, 0.0f), glm::mat4(1.0f),
    glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), textureName, 0, false);
}

void initRenderer(uint32_t width, uint32_t height) {

#if !defined(NDEBUG) 
  r = &small3d::Renderer::getInstance("small3d Tests", 1024, 768);
#else
  r = &small3d::Renderer::getInstance("small3d Tests");
#endif

}

int LoggerTest() {
  deleteLogger();
  
  initLogger();
  LOGINFO("Logger info test works");
  LOGERROR("Logger error test works");
  deleteLogger();
  return 1;
}

int ImageTest() {

  Image image(resourceDir + "/images/testImage.png");

  cout << "Image width " << image.getWidth() << ", height " <<
    image.getHeight() << endl;


  return 1;
}

int WavefrontFailTest() {

  WavefrontFile wf(resourceDir + "/models/goat.glb");
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
  Model model(WavefrontFile(resourceDir + "/models/Cube/Cube.obj"), "");

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

  Model modelWithNoTexture(WavefrontFile(resourceDir + "/models/Cube/CubeNoTexture.obj"), "");

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

  WavefrontFile w2(resourceDir + "/models/goatAndTree.obj");

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

  SceneObject boxes("boxes", Model(GlbFile(resourceDir + "/models/boxes.glb"), ""));

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

int GlbTextureTestDefaultShadows() {
  initRenderer();

  r->shadowsActive = true;

  double startSeconds = getTimeInSeconds();
  double seconds = getTimeInSeconds();
  double prevSeconds = seconds;
  const uint32_t framerate = 30;

  constexpr double secondsInterval = 1.0 / framerate;
  SceneObject goat("goat5", Model(GlbFile(resourceDir + "/models/goatAndTree.glb"), "Cube"));

  r->generateTexture("goatGlbTexture", *goat.getModel().defaultTextureImage);
  SceneObject tree("tree5", Model(GlbFile(resourceDir + "/models/goatAndTree.glb"), "Cube.001"));

  r->generateTexture("treeGlbTexture", *tree.getModel().defaultTextureImage);

  Model rect;

  r->createRectangle(rect, glm::vec3(-5.0f, -1.5f, -14.0f),
    glm::vec3(5.0f, -1.5f, 4.0f));

  goat.position = glm::vec3(-1.1f, -1.0f, -7.0f);
  goat.startAnimating();
  tree.position = glm::vec3(1.0f, -1.0f, -7.0f);
  auto rectPos = glm::vec3(0.0, 0.6, -5.6);
  auto rectRot = glm::vec3(0.0, 0.0, 0.0);

  while (seconds - startSeconds < 4.0) {

    pollEvents();
    seconds = getTimeInSeconds();

    if (seconds - prevSeconds > secondsInterval) {
      prevSeconds = seconds;
      goat.animate();

      r->render(rect, rectPos, rectRot, glm::vec4(0.7f, 0.7f, 0.7f, 1.0f));
      r->render(goat, "goatGlbTexture");

      goat.position = glm::vec3(1.1f, -1.0f, -7.0f);
      r->render(goat, "goatGlbTexture");
      goat.position = glm::vec3(-1.1f, -1.0f, -7.0f);

      r->render(tree, "treeGlbTexture");
      write("Shadows using simple transformation", 0.0f);
      r->swapBuffers();
      goat.rotate(glm::vec3(0.0f, 0.03f, 0.0f));
    }
  }

  r->shadowsActive = false;
  r->deleteTexture("goatGlbTexture");
  r->deleteTexture("treeGlbTexture");

  return 1;
}

int GlbTextureTestLookAtShadows() {
  initRenderer();

  r->shadowsActive = true;

  double startSeconds = getTimeInSeconds();
  double seconds = getTimeInSeconds();
  double prevSeconds = seconds;
  const uint32_t framerate = 30;

  constexpr double secondsInterval = 1.0 / framerate;

  SceneObject goat("goat5", Model(GlbFile(resourceDir + "/models/goatAndTree.glb"), "Cube"));

  r->generateTexture("goatGlbTexture", *goat.getModel().defaultTextureImage);

  SceneObject tree("tree5", Model(GlbFile(resourceDir + "/models/goatAndTree.glb"), "Cube.001"));

  r->generateTexture("treeGlbTexture", *tree.getModel().defaultTextureImage);

  Model rect;

  r->createRectangle(rect, glm::vec3(-5.0f, -1.5f, -14.0f),
    glm::vec3(5.0f, -1.5f, 4.0f));

  goat.position = glm::vec3(-1.1f, -1.0f, -7.0f);
  goat.startAnimating();
  tree.position = glm::vec3(1.0f, -1.0f, -7.0f);
  auto rectPos = glm::vec3(0.0, 0.6, -5.6);
  auto rectRot = glm::vec3(0.0, 0.0, 0.0);

  auto topForShadows = tree.position;
  topForShadows.y += 8.0f;
  topForShadows.z -= 0.1f; // for lookAt to work..
  auto up = glm::vec3(0.0f, 1.0f, 0.0f);

  r->shadowCamTransformation = glm::lookAt(topForShadows, tree.position, up);

  while (seconds - startSeconds < 4.0) {

    pollEvents();
    seconds = getTimeInSeconds();

    if (seconds - prevSeconds > secondsInterval) {
      prevSeconds = seconds;
      goat.animate();

      r->render(rect, rectPos, rectRot, glm::vec4(0.7f, 0.7f, 0.7f, 1.0f));
      r->render(goat, "goatGlbTexture");

      goat.position = glm::vec3(1.1f, -1.0f, -7.0f);
      r->render(goat, "goatGlbTexture");
      goat.position = glm::vec3(-1.1f, -1.0f, -7.0f);

      r->render(tree, "treeGlbTexture");
      write("Shadows using glm::lookAt", 0.0f);
      r->swapBuffers();
      goat.rotate(glm::vec3(0.0f, 0.03f, 0.0f));
    }
  }

  r->shadowsActive = false;
  r->deleteTexture("goatGlbTexture");
  r->deleteTexture("treeGlbTexture");
  return 1;
}

int BoundingBoxesTest() {

  initRenderer();

  double startSeconds = getTimeInSeconds();
  double seconds = getTimeInSeconds();
  double prevSeconds = seconds;
  const uint32_t framerate = 30;

  constexpr double secondsInterval = 1.0 / framerate;
  SceneObject goat("goat", Model(GlbFile(resourceDir + "/models/goatUnscaled.glb"), "Cube"), 3);
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

  return 1;
}

int FPStest() {

  initRenderer();

  double startSeconds = getTimeInSeconds();
  double seconds = getTimeInSeconds();
  double prevSeconds = seconds;
  uint32_t framerate = 0;
  uint32_t numFrames = 0;
  SceneObject goat("goat", Model(GlbFile(resourceDir + "/models/goatUnscaled.glb"), "Cube"), 3);
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

    goat.animate();

    r->render(goat, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

    for (auto& m : boundingBoxModels) {
      r->render(m, goat.position,
        goat.getTransformation(), glm::vec4(5.0f, 5.0f, 1.0f, 0.5f));
    }

    r->generateTexture("frameRate", std::to_string(framerate) + " FPS",
      glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));

    r->render(texturedRect,
      glm::vec3(0.0f, 0.0f, -1.0f),
      glm::vec3(0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), "frameRate", 0, false);

    r->swapBuffers();
    ++numFrames;
    goat.rotate(glm::vec3(0.0f, 0.01f, 0.0f));

  }

  return 1;
}

int GenericSceneObjectConstructorTest() {

  SceneObject so1("goat1", Model(GlbFile(resourceDir + "/models/goat.glb"), ""));
  SceneObject so2("goat2", Model(WavefrontFile(resourceDir + "/models/goat.obj"), ""));

  Model modelFromGlb(GlbFile(resourceDir + "/models/goatWithTexture.glb"), "");
  
  modelFromGlb.saveBinary("testGoatWithTexture1.bin");
  SceneObject so3("goat3", Model(BinaryFile("testGoatWithTexture1.bin"), ""));

  if (so1.getModel().vertexDataByteSize == 0) return 0;
  if (so2.getModel().vertexDataByteSize == 0) return 0;

  if (so3.getModel().vertexDataByteSize == 0) return 0;

  return 1;
}

int RendererTest() {
  initRenderer();

  r->setCameraRotation(glm::vec3(0.4f, 0.1f, 0.1f));

  // Here loading the mesh without providing a name is also tested.
  Model modelFromGlb(GlbFile(resourceDir + "/models/goatUnscaled.glb"), "");


  WavefrontFile cubef(resourceDir + "/models/Cube/CubeNoTexture.obj");
  SceneObject object("cube", cubef);
  object.position = glm::vec3(0.0f, -1.0f, -8.0f);
  r->render(object, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

  WavefrontFile texcubf(resourceDir + "/models/Cube/Cube.obj");
  SceneObject object2("texutredCube", texcubf);
  object2.position = glm::vec3(-2.0f, -1.0f, -7.0f);
  object2.setRotation(glm::vec3(0.3f, 1.3f, 0.0f));

  Image cubeTexture(resourceDir + "/models/Cube/cubeTexture.png");
  r->generateTexture("cubeTexture", cubeTexture);

  glfwShowWindow(r->getWindow());

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

  std::vector<std::shared_ptr<Model>> bugwfanim;

  bugwfanim.push_back(std::make_shared<Model>(WavefrontFile(resourceDir + "/models/Bug/bugAnim_000001.obj")));
  bugwfanim.push_back(std::make_shared<Model>(WavefrontFile(resourceDir + "/models/Bug/bugAnim_000002.obj")));
  bugwfanim.push_back(std::make_shared<Model>(WavefrontFile(resourceDir + "/models/Bug/bugAnim_000003.obj")));
  bugwfanim.push_back(std::make_shared<Model>(WavefrontFile(resourceDir + "/models/Bug/bugAnim_000004.obj")));
  bugwfanim.push_back(std::make_shared<Model>(WavefrontFile(resourceDir + "/models/Bug/bugAnim_000005.obj")));
  bugwfanim.push_back(std::make_shared<Model>(WavefrontFile(resourceDir + "/models/Bug/bugAnim_000006.obj")));
  bugwfanim.push_back(std::make_shared<Model>(WavefrontFile(resourceDir + "/models/Bug/bugAnim_000007.obj")));
  bugwfanim.push_back(std::make_shared<Model>(WavefrontFile(resourceDir + "/models/Bug/bugAnim_000008.obj")));
  bugwfanim.push_back(std::make_shared<Model>(WavefrontFile(resourceDir + "/models/Bug/bugAnim_000009.obj")));

  SceneObject bug("bug", bugwfanim);

  bug.position = glm::vec3(0.0f, 3.0f, -6.0f);
  bug.startAnimating();

  while (seconds - startSeconds < 5.0) {
    pollEvents();
    seconds = getTimeInSeconds();
    if (seconds - prevSeconds > secondsInterval) {
      prevSeconds = seconds;

      r->render(singleColourRect,
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), "", 0, false);

      r->render(texturedRect,
        glm::vec3(0.0f, 0.0f, -2.0f),
        glm::vec3(0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), "cubeTexture", true);

      r->render(object2, "cubeTexture");


      rotation.y += 0.1f;

      r->render(modelFromGlb, glm::vec3(0.0f, 1.0f, -6.0f),
        rotation, glm::vec4(0.3f, 1.0f, 1.0f, 1.0f));


      r->render(textRect, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), "small3dTexture", 0, false);

      r->render(bug, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

      bug.animate();

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

int BinaryModelTest() {

  initRenderer();

  r->setCameraRotation(glm::vec3(0.4f, 0.1f, 0.1f));
  const std::string textureName = "goatbintexture";

  Model modelFromGlb(GlbFile(resourceDir + "/models/goatWithTexture.glb"), "");

  modelFromGlb.saveBinary("testGoatWithTexture.bin");
  Model modelFromBin(BinaryFile("testGoatWithTexture.bin"), "");

  r->generateTexture(textureName, *modelFromBin.defaultTextureImage);
  double startSeconds = getTimeInSeconds();
  double seconds = getTimeInSeconds();
  double prevSeconds = seconds;
  const uint32_t framerate = 30;

  constexpr double secondsInterval = 1.0 / framerate;

  SceneObject so("goat", modelFromBin);

  so.position = glm::vec3(0.0f, 1.0f, -6.0f);
  so.startAnimating();

  glm::vec3 rotation(0.0f, 0.0f, 0.0f);
  while (seconds - startSeconds < 5.0) {
    pollEvents();
    seconds = getTimeInSeconds();
    if (seconds - prevSeconds > secondsInterval) {
      prevSeconds = seconds;

      so.rotate(glm::vec3(0.0f, 0.1f, 0.0f));
      so.animate();
      r->render(so, textureName);
      r->swapBuffers();
    }
  }

  return 1;
}

int SoundTest() {
  Sound snd(resourceDir + "/sounds/bah.ogg");
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
  while (getTimeInSeconds() - startSeconds < 2.0);
  return 1;
}

int BinSoundTest() {

  Sound srcsnd(resourceDir + "/sounds/bah.ogg");
  
  srcsnd.saveBinary("testBah.bin");
  Sound snd("testBah.bin");

  snd.play();
  double startSeconds = getTimeInSeconds();
  while (getTimeInSeconds() - startSeconds < 0.2);
  snd.stop();

  return 1;
}

int SoundTest2() {
  Sound snd1(resourceDir + "/sounds/bah.ogg");
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
  while (getTimeInSeconds() - startSeconds < 1.0);
  return 1;
}

int SoundTest3() {
  Sound snd(resourceDir + "/sounds/bah.ogg");
  snd.play(true);
  double startSeconds = getTimeInSeconds();
  while (getTimeInSeconds() - startSeconds < 6.0);
  return 1;
}

int GlbTest() {

  GlbFile glb(resourceDir + "/models/goat.glb");

  glb.printTokensRecursive();

  glb.printTokensSerial();

  return 1;
}

int ModelsTimeToLoad() {
  initLogger();
  auto startTime = getTimeInSeconds();
  Model glb(GlbFile(resourceDir + "/models/goat.glb"), "");
  LOGINFO("Read glTF model in " + std::to_string(getTimeInSeconds() - startTime) + " seconds.");

  startTime = getTimeInSeconds();
  Model wf(WavefrontFile(resourceDir + "/models/goat.obj"), "");
  LOGINFO("Read Wavefront model in " + std::to_string(getTimeInSeconds() - startTime) + " seconds.");

  return 1;
}

#ifdef _WIN32
int ScreenCaptureTest() {

  initRenderer();

  SceneObject boxes("boxes", Model(GlbFile(resourceDir + "/models/boxes.glb"), ""));
  boxes.position = glm::vec3(0.0f, 0.0f, -3.0f);
  r->render(boxes, glm::vec4(0.5f, 0.3f, 0.0f, 1.0f));
  r->swapBuffers();
  std::this_thread::sleep_for(std::chrono::seconds(1));
  r->render(boxes, glm::vec4(0.5f, 0.3f, 0.0f, 1.0f));
  r->screenCapture = true;
  r->swapBuffers();

  return 1;

}

int ControllerTest() {

  if (glfwJoystickPresent(GLFW_JOYSTICK_1)) {

    LOGINFO("Found joystick: " + std::string(glfwGetJoystickName(GLFW_JOYSTICK_1)));

    int axes_count;
    const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axes_count);
    LOGINFO("Axes count: " + std::to_string(axes_count));
    LOGINFO("Axes value: " + std::to_string(*axes));

    if (glfwJoystickIsGamepad(GLFW_JOYSTICK_1)) {
      
      LOGINFO("The joystick is a gamepad: " + std::string(glfwGetGamepadName(GLFW_JOYSTICK_1)));

      GLFWgamepadstate state;
      
      if (glfwGetGamepadState(GLFW_JOYSTICK_1, &state)) {

        LOGINFO("Retrieved gamepad state.");
      }

    }

  }

  return 1;

}

#endif
