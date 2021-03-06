//
//  interop.m
//  small3dios
//
//  Created by me on 26/09/2019.
//  Copyright © 2019 dimi309. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <CoreGraphics/CoreGraphics.h>
#import <MoltenVK/mvk_vulkan.h>

@import os.log;

static os_log_t _log;

void* app_window;

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

int get_app_width() {
  CGRect screenRect = [[UIScreen mainScreen] bounds];
  return screenRect.size.width;
}

int get_app_height() {
  CGRect screenRect = [[UIScreen mainScreen] bounds];
  return screenRect.size.height;
}

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
