/*
 *  main.cpp
 *
 *  Created on: 2014/10/18
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#include <gtest/gtest.h>

#include <small3d/Renderer.hpp>

#include <small3d/Logger.hpp>
#include <small3d/Image.hpp>
#include <small3d/Model.hpp>
#include <small3d/SceneObject.hpp>
#include <small3d/GetTokens.hpp>
#include <small3d/Sound.hpp>
#include <small3d/BoundingBoxSet.hpp>

using namespace small3d;
using namespace std;

TEST(LoggerTest, LogSomething) {
  deleteLogger();
  ostringstream oss;
  initLogger(oss);
  
  LOGINFO("It works");
  EXPECT_TRUE(oss.str().find("It works") != (string::npos));
  
  LOGERROR("Error test");
  EXPECT_TRUE(oss.str().find("Error test") != (string::npos));
  deleteLogger();
  
}

TEST(ImageTest, LoadImage) {
  
  Image image("resources/images/testImage.png");
  
  cout << "Image width " << image.getWidth() << ", height " <<
    image.getHeight() << endl;
  
  const float *imageData = image.getData();
  
  unsigned long x = 0, y = 0;
  
  while (y < image.getHeight()) {
    x = 0;
    while (x < image.getWidth()) {
      
      const float *colour = &imageData[4 * y * image.getWidth() + 4 * x];
      
      EXPECT_GE(colour[0], 0.0f);
      EXPECT_LE(colour[0], 1.0f);
      EXPECT_GE(colour[1], 0.0f);
      EXPECT_LE(colour[1], 1.0f);
      EXPECT_GE(colour[2], 0.0f);
      EXPECT_LE(colour[2], 1.0f);
      EXPECT_EQ(1.0f, colour[3]);
      
      ++x;
    }
    ++y;
  }
}

TEST(ModelTest, LoadModel) {
  
  Model model("resources/models/Cube/Cube.obj");
  
  EXPECT_NE(0, model.vertexData.size());
  EXPECT_NE(0, model.indexData.size());
  EXPECT_NE(0, model.normalsData.size());
  EXPECT_NE(0, model.textureCoordsData.size());
  
  cout << "Vertex data component count: "
       << model.vertexData.size() << endl << "Index count: "
       << model.indexData.size() << endl
       << "Normals data component count: "
       << model.normalsData.size() << endl
       << "Texture coordinates count: "
       << model.textureCoordsData.size() << endl;
  
  Model modelWithNoTexture("resources/models/Cube/CubeNoTexture.obj");
  
  EXPECT_NE(0, modelWithNoTexture.vertexData.size());
  EXPECT_NE(0, modelWithNoTexture.indexData.size());
  EXPECT_NE(0, modelWithNoTexture.normalsData.size());
  EXPECT_EQ(0, modelWithNoTexture.textureCoordsData.size());
  
  cout << "Vertex data component count: "
       << modelWithNoTexture.vertexData.size() << endl << "Index count: "
       << modelWithNoTexture.indexData.size() << endl
       << "Normals data component count: "
       << modelWithNoTexture.normalsData.size() << endl
       << "Texture coordinates count: "
       << modelWithNoTexture.textureCoordsData.size() << endl;
  
}

TEST(BoundingBoxesTest, LoadBoundingBoxes) {
  
  BoundingBoxSet bboxes("resources/models/GoatBB/GoatBB.obj");
  
  EXPECT_EQ(16, bboxes.vertices.size());
  EXPECT_EQ(12, bboxes.facesVertexIndexes.size());
  
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
  
  EXPECT_FALSE(bboxes.collidesWith(glm::vec3(0.1f, 0.1f, 0.1f),
				   glm::vec3(0.0f, 0.1f, 0.1f), 
				   glm::vec3(0.0f, 0.0f, 0.0f)));
  
}


TEST(RendererTest, StartAndUse) {

  Renderer *renderer = &Renderer::getInstance("test", 640, 480);
  renderer->clearScreen();
  
  SceneObject object("cube", "resources/models/Cube/CubeNoTexture.obj");
  object.offset = glm::vec3(0.0f, -1.0f, -8.0f);
  renderer->render(object, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

  renderer->renderRectangle(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
			    glm::vec3(-1.0f, 0.0f, 1.0f),
			    glm::vec3(-0.5f, -0.5f, 1.0f), false);

  SceneObject object2("texutredCube", "resources/models/Cube/Cube.obj");
  object2.offset = glm::vec3(-2.0f, -1.0f, -7.0f);
  object2.rotation = glm::vec3(0.3f, 1.3f, 0.0f);
  
  Image cubeTexture("resources/models/Cube/cubeTexture.png");
  renderer->generateTexture("cubeTexture", cubeTexture);

  renderer->render(object2, "cubeTexture");

  renderer->write("small3d :)", glm::vec3(0.0f, 1.0f, 0.0f),
		  glm::vec2(-1.0f, 0.0f), glm::vec2(0.5f, -0.5f));

  renderer->swapBuffers();
  
  renderer->deleteTexture("cubeTexture");
  
}



TEST(SoundTest, LoadAndPlay) {
  Sound snd("resources/sounds/bah.ogg");
  snd.play();
  double startSeconds = glfwGetTime();
  while(glfwGetTime() - startSeconds < 0.5);
  snd.stop();
  startSeconds = glfwGetTime();
  while(glfwGetTime() - startSeconds < 0.5);
  snd.play();
  // Make sure the sound is stopped by the stop function and not the destructor.
  startSeconds = glfwGetTime();
  while(glfwGetTime() - startSeconds < 2.0);
}


TEST(SoundTest, ThreeAtTheSameTime) {
  Sound snd1("resources/sounds/bah.ogg");
  Sound snd2(snd1);
  Sound snd3 = snd2;
  snd1.play();
  double startSeconds = glfwGetTime();
  while(glfwGetTime() - startSeconds < 0.3);
  snd2.play();
  startSeconds = glfwGetTime();
  while(glfwGetTime() - startSeconds < 0.3);
  snd3.play();
  startSeconds = glfwGetTime();
  while(glfwGetTime() - startSeconds < 1.0);
}


TEST(SoundTest, RepeatSound) {
  Sound snd("resources/sounds/bah.ogg");
  snd.play(true);
  double startSeconds = glfwGetTime();
  while(glfwGetTime() - startSeconds < 6.0);
}


TEST(TokenTest, GetFourTokens) {
  string strTest = "a-b-c-d";
  std::vector<std::string> tokens;
  
  int tokenCount=getTokens(strTest, '-', tokens);
  
  EXPECT_EQ(4, tokenCount);
  EXPECT_EQ("b", tokens[1]);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
