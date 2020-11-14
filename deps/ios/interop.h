//
//  interop.h
//  small3dios
//
//  Created by me on 26/09/2019.
//  Copyright © 2019 dimi309. All rights reserved.
//

#ifndef interop_h
#define interop_h
#include <MoltenVK/mvk_vulkan.h>

extern "C" {

extern void* app_window;

void init_log();

void log_info(const char *msg);

void log_debug(const char *msg);

void log_error(const char *msg);

const char* get_ios_base_path();

int get_app_width();

int get_app_height();

int create_ios_surface(VkInstance instance, VkSurfaceKHR *surface);

}

#endif /* interop_h */
