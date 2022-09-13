/*
 * Time.cpp
 *
 * Created on: 13 Sep 2022
 *     Author: Dimitri Kourkoulis
 *    License: BSD 3-Clause License (see LICENSE file)
 */

#include "Time.hpp"

#if !defined(__ANDROID__) && !defined(SMALL3D_IOS)
#include <GLFW/glfw3.h>
#else
#include <sys/time.h>
#endif

double getTimeInSeconds() {
#if !defined(__ANDROID__) && !defined(SMALL3D_IOS)
  return glfwGetTime();
#else
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (double) tv.tv_sec + tv.tv_usec / 1000000.0f;
#endif
}
