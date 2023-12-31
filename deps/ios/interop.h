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

#ifdef SMALL3D_USING_XCODE
// Needed for compiling within Xcode
extern "C" {
#endif

void init_log();

void log_info(const char *msg);

void log_debug(const char *msg);

void log_error(const char *msg);

const char* get_ios_base_path();

const char* get_ios_writeable_path();

int get_app_width();

int get_app_height();

#ifdef SMALL3D_USING_XCODE
// Needed for compiling within Xcode (see extern "C" at the top of the file)
}
#endif

#endif /* interop_h */
