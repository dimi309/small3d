/*
 *  main.cpp
 *
 *  Created on: 2014/10/18
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */
#include <iostream>
#include <small3d/Renderer.hpp>

#include <small3d/Logger.hpp>
#include <small3d/Image.hpp>
#include <small3d/Model.hpp>
#include <small3d/SceneObject.hpp>
#include <small3d/Sound.hpp>
#include <small3d/BoundingBoxSet.hpp>
#include <small3d/GlbFile.hpp>
#include <small3d/WavefrontFile.hpp>

using namespace small3d;
using namespace std;

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

  WavefrontFile wf("resources/models/GoatBB/GoatBB.obj");
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
  WavefrontFile w("resources/models/Cube/Cube.obj");
  Model model(&w, "");

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

  WavefrontFile w1("resources/models/Cube/CubeNoTexture.obj");
  Model modelWithNoTexture(&w1, "");

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

  Model model2(&w2, "Cube.001");
  Model model3(&w2, "Cube");
  Model model4(&w2, "");

  Renderer* renderer = &Renderer::getInstance("test", 640, 480, 0.785f, 1.0f, 24.0f, "resources/shaders/", 1000);

  double startSeconds = glfwGetTime();
  double seconds = glfwGetTime();
  double prevSeconds = seconds;
  const uint32_t framerate = 30;

  constexpr double secondsInterval = 1.0 / framerate;

  while (seconds - startSeconds < 5.0) {
    glfwPollEvents();
    seconds = glfwGetTime();
    if (seconds - prevSeconds > secondsInterval) {
      renderer->clearScreen();
      
      renderer->render(model2, glm::vec3(-1.5f, -1.0f, -3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
      renderer->render(model3, glm::vec3(0.0f, -1.0f, -2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
      renderer->render(model4, glm::vec3(1.5f, -1.0f, -3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));

      renderer->swapBuffers();
    }
  }

  return 1;
}

int GlbTextureTest() {
  Renderer* renderer = &Renderer::getInstance("test", 640, 480, 0.785f, 1.0f, 24.0f, "resources/shaders/", 1000);

  double startSeconds = glfwGetTime();
  double seconds = glfwGetTime();
  double prevSeconds = seconds;
  const uint32_t framerate = 30;

  constexpr double secondsInterval = 1.0 / framerate;

  SceneObject goat("goat5", "resources/models/goatAndTree.glb", "Cube");

  renderer->generateTexture("goatGlbTexture", *goat.getModel().defaultTextureImage);

  SceneObject tree("tree5", "resources/models/goatAndTree.glb", "Cube.001");

  renderer->generateTexture("treeGlbTexture", *tree.getModel().defaultTextureImage);

  goat.offset = glm::vec3(0.0f, 0.0f, -3.0f);
  goat.startAnimating();
  tree.offset = glm::vec3(0.0f, 0.0f, -4.0f);


  while (seconds - startSeconds < 5.0) {
    glfwPollEvents();
    seconds = glfwGetTime();
    if (seconds - prevSeconds > secondsInterval) {
      renderer->clearScreen();
      goat.animate();

      renderer->render(goat, "goatGlbTexture");
      renderer->render(tree, "treeGlbTexture");

      renderer->swapBuffers();
      goat.rotation.y += 0.01f;
    }
  }

  return 1;
}

int BoundingBoxesTest() {
  
  BoundingBoxSet bboxes("resources/models/GoatBB/GoatBB.obj");
  
  if (bboxes.vertices.size() != 16) return 0;
  if (bboxes.facesVertexIndexes.size() != 12) return 0;
  if (bboxes.facesVertexIndexesTriangulated.size() != 24) return 0;

  cout << "Bounding boxes vertices: " << endl;
  for (unsigned long idx = 0; idx < 16; idx++) {
    cout << bboxes.vertices[idx][0] << ", " <<
      bboxes.vertices[idx][1] << ", " <<
      bboxes.vertices[idx][2] << ", " << endl;
  }

  cout << "Bounding boxes faces vertex indexes: " << endl;
  for (unsigned long idx = 0; idx < 12; idx++) {
    cout << bboxes.facesVertexIndexes[idx][0] << ", " <<
      bboxes.facesVertexIndexes[idx][1] << ", " <<
      bboxes.facesVertexIndexes[idx][2] << ", " <<
      bboxes.facesVertexIndexes[idx][3] << ", " << endl;
  }

  if (bboxes.contains(glm::vec3(0.1f, 0.1f, 0.1f),
    glm::vec3(0.0f, 0.1f, 0.1f),
    glm::vec3(0.0f, 0.0f, 0.0f))) {

    return 0;
  }

  Renderer* renderer = &Renderer::getInstance("test", 640, 480, 0.785f, 1.0f, 24.0f, "resources/shaders/", 1000);

  double startSeconds = glfwGetTime();
  double seconds = glfwGetTime();
  double prevSeconds = seconds;
  const uint32_t framerate = 30;

  constexpr double secondsInterval = 1.0 / framerate;

  SceneObject goat("goat", "resources/models/goatUnscaled.glb", "Cube", 3);
  auto boundingBoxModels = goat.boundingBoxSet.getModels();
  goat.offset = glm::vec3(0.0f, 0.0f, -3.0f);
  goat.startAnimating();

  while (seconds - startSeconds < 5.0) {
    glfwPollEvents();
    seconds = glfwGetTime();
    if (seconds - prevSeconds > secondsInterval) {
      renderer->clearScreen();
      goat.animate();

      renderer->render(goat, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

      for (auto& m : boundingBoxModels) {
        renderer->render(m, goat.offset,
          goat.rotation, glm::vec4(5.0f, 5.0f, 1.0f, 0.5f));
      }
      renderer->swapBuffers();
      goat.rotation.y += 0.01f;
    }
  }

  return 1;
}

int RendererTest() {
  Renderer* renderer = &Renderer::getInstance("test", 640, 480);

  renderer->cameraRotation = glm::vec3(0.4f, 0.1f, 0.1f);

  GlbFile g("resources/models/goatUnscaled.glb");
  // Here loading the mesh without providing a name is also tested.
  Model modelFromGlb(&g, ""); 

  SceneObject object("cube", "resources/models/Cube/CubeNoTexture.obj");
  object.offset = glm::vec3(0.0f, -1.0f, -8.0f);
  renderer->render(object, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

  SceneObject object2("texutredCube", "resources/models/Cube/Cube.obj");
  object2.offset = glm::vec3(-2.0f, -1.0f, -7.0f);
  object2.rotation = glm::vec3(0.3f, 1.3f, 0.0f);

  Image cubeTexture("resources/models/Cube/cubeTexture.png");
  renderer->generateTexture("cubeTexture", cubeTexture);

  glfwShowWindow(renderer->getWindow());

  Model singleColourRect;
  renderer->createRectangle(singleColourRect, glm::vec3(-1.0f, 0.0f, 0.0f),
    glm::vec3(-0.5f, -0.5f, 0.0f));

  Model texturedRect;

  renderer->createRectangle(texturedRect, glm::vec3(0.0f, 0.5f, 0.0f),
    glm::vec3(1.0f, -1.0f, 0.0f));

  renderer->generateTexture("small3dTexture", "small3d :)",
    glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));

  Model textRect;

  renderer->createRectangle(textRect, glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.5f, -0.5f, 0.0f));

  double startSeconds = glfwGetTime();
  double seconds = glfwGetTime();
  double prevSeconds = seconds;
  const uint32_t framerate = 30;

  constexpr double secondsInterval = 1.0 / framerate;

  glm::vec3 rotation(0.0f, 0.0f, 0.0f);

  while (seconds - startSeconds < 5.0) {
    glfwPollEvents();
    seconds = glfwGetTime();
    if (seconds - prevSeconds > secondsInterval) {
      renderer->clearScreen();

      renderer->render(singleColourRect,
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), "", false);

      renderer->generateTexture("small3dTexture", std::to_string(modelFromGlb.getCurrentPoseIdx()),
        glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));

      renderer->render(texturedRect,
        glm::vec3(0.0f, 0.0f, -2.0f),
        glm::vec3(0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), "cubeTexture", true);

      renderer->render(object2, "cubeTexture");

      modelFromGlb.animate();
      rotation.y += 0.1f;

      renderer->render(modelFromGlb, glm::vec3(0.0f, 1.0f, -6.0f),
        rotation, glm::vec4(0.3f, 1.0f, 1.0f, 1.0f));

      renderer->render(textRect, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), "small3dTexture", false);

      renderer->swapBuffers();
      prevSeconds = seconds;
    }
  }
  renderer->clearBuffers(object);
  renderer->clearBuffers(object2);
  renderer->clearBuffers(texturedRect);
  renderer->clearBuffers(textRect);
  renderer->deleteTexture("cubeTexture");

  return 1;
}

int SoundTest() {
  Sound snd("resources/sounds/bah.ogg");
  snd.play();
  double startSeconds = glfwGetTime();
  while (glfwGetTime() - startSeconds < 0.5);
  snd.stop();
  startSeconds = glfwGetTime();
  while (glfwGetTime() - startSeconds < 0.5);
  snd.play();
  // Make sure the sound is stopped by the stop function and not the destructor.
  startSeconds = glfwGetTime();
  while(glfwGetTime() - startSeconds < 2.0);
  return 1;
}

int SoundTest2() {
  Sound snd1("resources/sounds/bah.ogg");
  Sound snd2(snd1);
  Sound snd3 = snd2;
  snd1.play();
  double startSeconds = glfwGetTime();
  while (glfwGetTime() - startSeconds < 0.3);
  snd2.play();
  startSeconds = glfwGetTime();
  while (glfwGetTime() - startSeconds < 0.3);
  snd3.play();
  startSeconds = glfwGetTime();
  while(glfwGetTime() - startSeconds < 1.0);
  return 1;
}

int SoundTest3() {
  Sound snd("resources/sounds/bah.ogg");
  snd.play(true);
  double startSeconds = glfwGetTime();
  while(glfwGetTime() - startSeconds < 6.0);
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

int main(int argc, char** argv) {
  try
  {
    if (!LoggerTest()) {
      printf("*** Failing LoggerTest.\n\r");
      return 1;
    }

    if (!ImageTest()) {
      printf("*** Failing ImageTest.\n\r");
      return 1;
    }

    if (!WavefrontTest()) {
      printf("*** Failing WavefrontTest.\n\r");
      return 1;
    }

    if (!WavefrontModelTest()) {
      printf("*** Failing WavefrontModelTest.\n\r");
      return 1;
    }

    if (!GlbTextureTest()) {
      printf("*** Failing GlbTextureText.\n\r");
      return 1;
    }

    if (!BoundingBoxesTest()) {
      printf("*** Failing BoundingBoxesTest.\n\r");
      return 1;
    }

    if (!GenericSceneObjectConstructorTest()) {
      printf("*** Failing GenericSceneObjectConstructorTest.\n\r");
      return 1;
    }

    if (!RendererTest()) {
      printf("*** Failing RendererTest.\n\r");
      return 1;
    }

    if (!SoundTest()) {
      printf("*** Failing SoundTest.\n\r");
      return 1;
    }

    if (!SoundTest2()) {
      printf("*** Failing SoundTest2.\n\r");
      return 1;
    }

    if (!SoundTest3()) {
      printf("*** Failing SoundTest3.\n\r");
      return 1;
    }

    if (!GlbTest()) {
      printf("*** Failing GlbTest.\n\r");
      return 1;
    }

  }
  catch (exception& e) {
    printf("*** %s\n\r", e.what());
    return 1;
  }
  printf("All tests have executed successfully.\n\r");
  return 0;
}
