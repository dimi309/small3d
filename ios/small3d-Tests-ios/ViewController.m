//
//  ViewController.m
//  islet-hell-ios
//
//  Created by me on 07/01/2022.
//

#import "ViewController.h"
#include "interop.h"

#include <small3d/Renderer.hpp>

small3d::Renderer *r;
std::string resourceDir = "resources1";

@implementation ViewController {
  CADisplayLink* _displayLink;
}

- (void)viewDidLoad {
  [super viewDidLoad];
    
    
  // Do any additional setup after loading the view.
  app_window = (__bridge void*) self.view.layer;
  
    
  small3d::initLogger();
    
    
  try {
    r = &small3d::Renderer::getInstance("Islet Hell", 854, 480, 0.785f, 1.0f, 24.0f,
        resourceDir + "/shaders/", 5000);
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
// ***RENDER HERE
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




