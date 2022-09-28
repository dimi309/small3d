//
//  ViewController.h
//  
//
//  Created by me on 07/01/2022.
//

#import <GLKit/GLKit.h>

@interface ViewController : GLKViewController

@property GLuint app_colour_render_buffer;
@property GLuint app_framebuffer;
@property GLuint app_depth_buffer;
@property CAEAGLLayer* app_eaglLayer;
@property GLKView* app_view;

@end

