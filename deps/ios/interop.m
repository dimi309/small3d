/**
 * @file  interop.m
 * @brief Utility functions for running small3d on ios
 *
 *  Created on: 2019/09/26
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <CoreGraphics/CoreGraphics.h>
#ifndef SMALL3D_OPENGL
#import <MoltenVK/mvk_vulkan.h>
#endif
#import <OSLog/OSLog.h>

#ifdef SMALL3D_USING_XCODE
// Needed for compiling within Xcode
extern "C" {
#endif
  
  static os_log_t _log;
  
  void init_log() {
    _log = os_log_create("small3d", "small3d");
  }
  
  void log_info(const char *msg) {
    os_log_info(_log, "%s", msg);
  }
  
  void log_debug(const char *msg) {
    os_log_debug(_log, "%s", msg);
  }
  
  void log_error(const char *msg) {
    os_log_error(_log, "%s", msg);
  }
  
  const char* get_ios_base_path() {
    
    NSBundle *mainBundle = [NSBundle mainBundle];
    
    return mainBundle.bundlePath.fileSystemRepresentation;
    
  }
  
  const char* get_ios_writeable_path() {
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0];
    return [documentsDirectory cStringUsingEncoding:[NSString defaultCStringEncoding]];
  }
  
  int get_app_width() {
    CGRect screenRect = [[UIScreen mainScreen] bounds];
    return screenRect.size.width;
  }
  
  int get_app_height() {
    CGRect screenRect = [[UIScreen mainScreen] bounds];
    return screenRect.size.height;
  }
#ifndef SMALL3D_OPENGL
  void* app_window;
  
  int create_ios_surface(VkInstance instance, VkSurfaceKHR *surface) {
    VkIOSSurfaceCreateInfoMVK surface_ci;
    surface_ci.sType = VK_STRUCTURE_TYPE_IOS_SURFACE_CREATE_INFO_MVK;
    surface_ci.pNext = NULL;
    surface_ci.flags = 0;
    surface_ci.pView = app_window;
    
    if (vkCreateIOSSurfaceMVK(instance, &surface_ci, NULL, surface) !=
        VK_SUCCESS) {
      return 0;
    }
    
    return 1;
  }
#endif
  
#ifdef SMALL3D_USING_XCODE
// Needed for compiling within Xcode (see extern "C" at the top of the file)
}
#endif
