#include <android_native_app_glue.h>
#include <jni.h>
#include <Logger.hpp>
#include <iostream>
#include <android/asset_manager.h>
#include <android/configuration.h>
#include <unistd.h>
#include "BoundingBoxSet.hpp"
#include "Renderer.hpp"
#include "Image.hpp"
#include "Model.hpp"
#include "Sound.hpp"
#include "GetTokens.hpp"
#include <sys/time.h>

double currentTimeInSeconds()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + tv.tv_usec / 1000000;
}

extern "C" {
#include "vkzos.h"
}
using namespace small3d;
using namespace std;

Renderer *renderer;
SceneObject *object, *object2;

static bool rendererActive = false;

extern "C" {

int BoundingBoxesTest() {

  BoundingBoxSet bboxes("resources/models/GoatBB/GoatBB.obj");

  if (bboxes.vertices.size() != 16) return 0;
  if (bboxes.facesVertexIndexes.size() != 12) return 0;

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

  if (bboxes.collidesWith(glm::vec3(0.1f, 0.1f, 0.1f),
                          glm::vec3(0.0f, 0.1f, 0.1f),
                          glm::vec3(0.0f, 0.0f, 0.0f))) {

    return 0;
  }
  LOGDEBUG("Bounding boxes test succeeded.");
  return 1;
}

int ModelTest() {

  Model model("resources/models/Cube/Cube.obj");

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

  Model modelWithNoTexture("resources/models/Cube/CubeNoTexture.obj");

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

  LOGINFO("Model test succeeded.");
  return 1;
}

int ImageTest() {

  Image image("resources/images/testImage.png");

  cout << "Image width " << image.getWidth() << ", height " <<
       image.getHeight() << endl;

  const float *imageData = image.getData();

  unsigned long x = 0, y = 0;

  while (y < image.getHeight()) {
    x = 0;
    while (x < image.getWidth()) {

      const float *colour = &imageData[4 * y * image.getWidth() + 4 * x];

      if (colour[0] < 0.0f) return 0;
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
  LOGDEBUG("Image test succeeded.");
  return 1;
}

int initRenderer() {

  renderer = new Renderer("test");

  renderer->cameraRotation = glm::vec3(0.4f, 0.1f, 0.1f);

  object = new SceneObject("cube", "resources/models/Cube/CubeNoTexture.obj");
  object2 = new SceneObject("texutredCube", "resources/models/Cube/Cube.obj");

  object->offset = glm::vec3(0.0f, -1.0f, -8.0f);
  //renderer->render(*object, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

  object2->offset = glm::vec3(-2.0f, -1.0f, -7.0f);
  object2->rotation = glm::vec3(0.3f, 1.3f, 0.0f);

  Image cubeTexture("resources/models/Cube/cubeTexture.png");
  renderer->generateTexture("cubeTexture", cubeTexture);

  double startSeconds = currentTimeInSeconds();

  return 1;
}

void drawFrame() {

  renderer->renderRectangle(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
                            glm::vec3(-1.0f, 0.0f, 1.0f),
                            glm::vec3(-0.5f, -0.5f, 1.0f), false);

  renderer->renderRectangle("cubeTexture",
                            glm::vec3(0.0f, 0.5f, -2.0f),
                            glm::vec3(1.0f, -1.0f, -2.0f), true);

  renderer->render(*object2, "cubeTexture");

  renderer->write("small3d :) p q", glm::vec3(0.0f, 1.0f, 0.0f),
                  glm::vec2(-1.0f, 0.0f), glm::vec2(0.5f, -0.5f));
  renderer->swapBuffers();


}

void shutdownRenderer() {

  renderer->clearBuffers(*object);
  renderer->clearBuffers(*object2);
  delete object;
  delete object2;
  renderer->deleteTexture("cubeTexture");
  delete renderer;

};

int SoundTest() {
  Sound snd("resources/sounds/bah.ogg");
  snd.play();
  double startSeconds = currentTimeInSeconds();
  while (currentTimeInSeconds() - startSeconds < 0.5);
  snd.stop();
  startSeconds = currentTimeInSeconds();
  while (currentTimeInSeconds() - startSeconds < 0.5);
  snd.play();
  // Make sure the sound is stopped by the stop function and not the destructor.
  startSeconds = currentTimeInSeconds();

  while (currentTimeInSeconds() - startSeconds < 2.0);

  return 1;
}

int SoundTest2() {
  Sound snd1("resources/sounds/bah.ogg");
  Sound snd2(snd1);
  Sound snd3 = snd2;
  snd1.play();
  double startSeconds = currentTimeInSeconds();
  while (currentTimeInSeconds() - startSeconds < 0.3);
  snd2.play();
  startSeconds = currentTimeInSeconds();
  while (currentTimeInSeconds() - startSeconds < 0.3);
  snd3.play();
  startSeconds = currentTimeInSeconds();

  while (currentTimeInSeconds() - startSeconds < 1.0);

  return 1;
}

int SoundTest3() {
  Sound snd("resources/sounds/bah.ogg");
  snd.play(true);
  double startSeconds = currentTimeInSeconds();

  while (currentTimeInSeconds() - startSeconds < 6.0);

  return 1;
}

int TokenTest() {
  string strTest = "a-b-c-d";
  std::vector<std::string> tokens;

  int tokenCount = getTokens(strTest, '-', tokens);

  if (tokenCount != 4) return 0;
  if (tokens[1] != "b") return 0;
  return 1;
}

void handle_cmd(android_app *pApp, int32_t cmd) {
  switch (cmd) {
    case APP_CMD_INIT_WINDOW:
      AConfiguration_setOrientation(pApp->config,
        ACONFIGURATION_ORIENTATION_PORT);

      break;



    case APP_CMD_TERM_WINDOW:

      break;

    case APP_CMD_GAINED_FOCUS:
      if (!rendererActive) {
        initRenderer();
        rendererActive = true;
        drawFrame();
        BoundingBoxesTest();
        ModelTest();
        ImageTest();
        SoundTest();
        SoundTest2();
        SoundTest3();
        TokenTest();
      }
      break;

    case APP_CMD_LOST_FOCUS:
      rendererActive = false;
      shutdownRenderer();
      break;

    default:
      LOGERROR("event not handled: " + intToStr(cmd));
  }
}

int32_t handle_input(android_app* app, AInputEvent* event) {
  LOGDEBUG("Input event");
  return 1;
}

void android_main(struct android_app *state) {

  vkz_android_app = state;

  state->onAppCmd = handle_cmd;
  state->onInputEvent = handle_input;

  initLogger();

  int events;
  android_poll_source *pSource;
  do {
    if (ALooper_pollAll(0, nullptr, &events, (void **) &pSource) >= 0) {
      if (pSource != NULL) {
        pSource->process(state, pSource);
      }
    }

    if (rendererActive) {
      drawFrame();
    }

  } while (!state->destroyRequested);


  LOGINFO("Done.");
}
}