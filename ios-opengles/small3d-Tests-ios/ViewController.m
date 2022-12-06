//
//  ViewController.m
//
//  Created by me on 07/01/2022.
//

#import "ViewController.h"
#include "interop.h"
#include "UnitTests.hpp"
#include <small3d/Renderer.hpp>
#include <small3d/Model.hpp>
#include <small3d/SceneObject.hpp>
#include <small3d/WavefrontFile.hpp>
#include <small3d/GlbFile.hpp>

#if defined(SMALL3D_OPENGL)
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#endif

small3d::Model texturedRect;
small3d::Model goat(small3d::GlbFile("resources1/models/goatAndTree.glb"), "Cube");

@implementation ViewController {
  CADisplayLink* _displayLink;
}
-(void) generateBuffers: (GLuint&) framembuffer : (GLuint &)colourbuffer : (GLuint &)depthbuffer : (GLint&) width : (GLint&) height {
  
  // Ensuring that the buffer id parameter is zero when generating buffers.
  GLuint tmp = 0;
  
  glGenFramebuffers(1, &tmp);
  framembuffer = tmp;
  glBindFramebuffer(GL_FRAMEBUFFER, framembuffer);
  
  tmp = 0;
  glGenRenderbuffers(1, &tmp);
  colourbuffer = tmp;
  glBindRenderbuffer(GL_RENDERBUFFER, colourbuffer);
  
  [[self.app_view context] renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer *)self.app_eaglLayer];
  
  
  glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
  glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
  
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colourbuffer);
  
  tmp = 0;
  glGenRenderbuffers(1, &tmp);
  depthbuffer = tmp;
  glBindRenderbuffer(GL_RENDERBUFFER, depthbuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, self.app_depth_buffer);
  
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
          LOGERROR("Failed to make complete framebuffer object");
  }
  else {
    LOGDEBUG("Framebuffer object is complete.");
  }
  
}
- (void)viewDidLoad {
  [super viewDidLoad];
  
  small3d::initLogger();
  
  self.app_eaglLayer = (CAEAGLLayer*) self.view.layer;
  
  self.app_eaglLayer.opaque = YES;
  self.app_eaglLayer.drawableProperties =
  [NSDictionary dictionaryWithObjectsAndKeys:
                                      [NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
  
  self.app_view = [GLKView alloc];//(GLKView *)self.view;
  self.app_view.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
  
  if (!self.app_view.context) {
    LOGERROR("Failed to create OpenGL ES context.");
  }
  if(![EAGLContext setCurrentContext:self.app_view.context]) {
    LOGERROR("Failed to set current OpenGL ES context.");
  }
  
  GLint framebufferWidth = 0, framebufferHeight = 0;
  GLuint framebuffer, colourbuffer, depthbuffer;
  [self generateBuffers:framebuffer :colourbuffer :depthbuffer :framebufferWidth :framebufferHeight];
  self.app_framebuffer = framebuffer;
  self.app_colour_render_buffer = colourbuffer;
  self.app_depth_buffer = depthbuffer;
  
  
  LOGDEBUG("COLOUR BUFFER " + std::to_string(colourbuffer));

  try {
    LOGDEBUG("initialising renderer");
    initRenderer(framebufferWidth, framebufferHeight);
    r->origRenderbuffer = self.app_colour_render_buffer;
    r->origFramebuffer = self.app_framebuffer;
    r->shadowsActive = false;
    r->setBackgroundColour(glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
    LOGDEBUG("renderer initialised");
    r->createRectangle(texturedRect, glm::vec3(-0.8f, 0.1f, -1.0f),
      glm::vec3(0.8f, -0.1f, -1.0f));
    r->generateTexture("message_ios", "No extended testing due to ios render loop control", glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));

    SoundTest();
    
  }
  catch (std::exception& e) {
    LOGERROR(std::string(e.what()));
  }
    
  _displayLink = [CADisplayLink displayLinkWithTarget: self selector: @selector(renderLoop)];
  // This does not work on old iOSes
  // _displayLink.preferredFramesPerSecond = 60;
  [_displayLink addToRunLoop: NSRunLoop.currentRunLoop forMode: NSDefaultRunLoopMode];
    
}

- (void) dealloc {
  GLuint tmp = 0;
  
  if (self.app_framebuffer) {
    tmp = self.app_framebuffer;
    glDeleteFramebuffers(1, &tmp);
    self.app_framebuffer = 0;
  }
  
  if (self.app_depth_buffer) {
    tmp = self.app_depth_buffer;
    glDeleteRenderbuffers(1, &tmp);
    self.app_depth_buffer = 0;
  }
  
  if (self.app_colour_render_buffer) {
    tmp = self.app_colour_render_buffer;
    glDeleteRenderbuffers(1, &tmp);
    self.app_colour_render_buffer = 0;
  }
  
  if ([EAGLContext currentContext] == [self.app_view context]) {
    [EAGLContext setCurrentContext:nil];
  }
}

- (void) viewDidAppear:(BOOL)animated {
    
}

- (void) viewWillDisappear:(BOOL)animated {
    
}

-(void) renderLoop {
  
  glBindFramebuffer(GL_FRAMEBUFFER, self.app_framebuffer);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  r->render(goat, glm::vec3(-1.5f, -1.0f, -3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
  r->render(goat, glm::vec3(-1.5f, 1.0f, -4.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
  r->render(texturedRect, "message_ios");
  r->swapBuffers();
  const GLenum discards[]  = {GL_DEPTH_ATTACHMENT};
  glDiscardFramebufferEXT(GL_FRAMEBUFFER,1,discards);
  
  glBindRenderbuffer(GL_RENDERBUFFER, self.app_colour_render_buffer);
  if(![[self.app_view context] presentRenderbuffer:GL_RENDERBUFFER]) {
    LOGERROR("Could not present render buffer!");
  }
  
}

void processPhase(UITouchPhase phase, CGPoint touchPoint) {
  
}

- (void)touchesBegan:(NSSet<UITouch *> *)touches
           withEvent:(UIEvent *)event {
  for (UITouch *touch in [event allTouches]) {
    CGPoint touchPoint = [touch locationInView:self.view];
    
    processPhase([touch phase], touchPoint);
  }
}

- (void)touchesMoved:(NSSet<UITouch *> *)touches
           withEvent:(UIEvent *)event {
  for (UITouch *touch in [event allTouches]) {
    CGPoint touchPoint = [touch locationInView:self.view];
    
    processPhase([touch phase], touchPoint);
  }
  
}

- (void)touchesEnded:(NSSet<UITouch *> *)touches
           withEvent:(UIEvent *)event {
  for (UITouch *touch in [event allTouches]) {
    CGPoint touchPoint = [touch locationInView:self.view];
    
    processPhase([touch phase], touchPoint);
  }
}


@end




