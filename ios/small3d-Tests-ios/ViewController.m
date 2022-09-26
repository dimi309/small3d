//
//  ViewController.m
//  islet-hell-ios
//
//  Created by me on 07/01/2022.
//

#import "ViewController.h"
#include "interop.h"
#include "UnitTests.hpp"
#include <small3d/Renderer.hpp>
#include <small3d/Model.hpp>

small3d::Model texturedRect;

@implementation ViewController {
  CADisplayLink* _displayLink;
}

- (void)viewDidLoad {
  [super viewDidLoad];
    
    
  // Do any additional setup after loading the view.
  app_window = (__bridge void*) self.view.layer;
  
    
  small3d::initLogger();
    
    
  try {
    initRenderer();
    r->createRectangle(texturedRect, glm::vec3(-1.0f, 0.1f, -1.0f),
      glm::vec3(1.0f, -0.1f, -1.0f));
    r->generateTexture("message_ios", "No extended testing due to ios 'game' loop control", glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
    SoundTest();
    
  }
  catch (std::exception& e) {
    LOGERROR(std::string(e.what()));
  }
    
  _displayLink = [CADisplayLink displayLinkWithTarget: self selector: @selector(renderLoop)];
  _displayLink.preferredFramesPerSecond = 60;
  //[_displayLink preferredFramesPerSecond: 60 ];
  [_displayLink addToRunLoop: NSRunLoop.currentRunLoop forMode: NSDefaultRunLoopMode];
    
}

- (void) viewDidAppear:(BOOL)animated {
    
}

- (void) viewWillDisappear:(BOOL)animated {
    
}

-(void) renderLoop {
  
  r->render(texturedRect, "message_ios");
  r->swapBuffers();
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

@implementation View
+(Class) layerClass { return [CAMetalLayer class]; }
@end




