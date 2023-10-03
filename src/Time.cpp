/*
 * Time.cpp
 *
 * Created on: 13 Sep 2022
 *     Author: Dimitri Kourkoulis
 *    License: BSD 3-Clause License (see LICENSE file)
 */

#include "Time.hpp"

#if !defined(__ANDROID__) && !defined(SMALL3D_IOS)
#include <chrono>
#else
#include <sys/time.h>
#endif

double getTimeInSeconds() {
#if !defined(__ANDROID__) && !defined(SMALL3D_IOS)
  return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() / 1000.0;
#else
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (double) tv.tv_sec + tv.tv_usec / 1000000.0f;
#endif
}
