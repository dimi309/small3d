/**
 * @file UnitTests.hpp
 * @brief Unit tests
 *
 * Created on: 2022/09/26
 *    Author: Dimitri Kourkoulis
 *    License: BSD 3-Clause License (see LICENSE file)
 */

#include "Renderer.hpp"
extern small3d::Renderer *r;
void pollEvents();
void initRenderer(uint32_t width = 854, uint32_t height = 480);

int LoggerTest();
int MathTest();
int ImageTest();
int WavefrontFailTest();
int WavefrontModelTest();
int ScaleAndTransformTest();
int GlbTextureTestDefaultShadows();
int BoundingBoxesTest();
int FPStest();
int GenericSceneObjectConstructorTest();
int RendererTest();
int BinaryModelTest();
int SoundTest();
int BinSoundTest();
int SoundTest2();
int SoundTest3();
int GlbTest();
int ModelsTimeToLoad();
#ifdef _WIN32
int ScreenCaptureTest();
int ControllerTest();
#endif
