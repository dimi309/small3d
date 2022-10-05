/*
 *  UnitTests.hpp
 *
 *  Created on: 2022/09/26
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#ifdef __ANDROID__
#include "small3d_android.h"
#endif
#include "Renderer.hpp"
extern small3d::Renderer *r;
void pollEvents();
void initRenderer(uint32_t width = 854, uint32_t height = 480);

#ifdef __ANDROID__
extern "C" {
void get_screen_info();
void handle_cmd(android_app *pApp, int32_t cmd);
}

extern bool appActive;
extern bool instantiated;
extern uint32_t screenWidth, screenHeight;

#endif

int LoggerTest();
int ImageTest();
int WavefrontTest();
int WavefrontModelTest();
int ScaleAndTransformTest();
int GlbTextureTest();
int BoundingBoxesTest();
int FPStest();
int GenericSceneObjectConstructorTest();
int RendererTest();
int SoundTest();
int SoundTest2();
int SoundTest3();
int GlbTest();
