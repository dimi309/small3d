/**
 * @file  interop.h
 * @brief Utility functions header file for running small3d on ios
 *
 *  Created on: 2019/09/26
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

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
