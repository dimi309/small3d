/*
 * BasePath.cpp
 *
 * Created on: 31 Jan 2020
 *     Author: Dimitri Kourkoulis
 *    License: BSD 3-Clause License (see LICENSE file)
 */

#include "BasePath.hpp"

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

std::string getBasePath() {

  std::string basePath = "";

#ifdef __APPLE__

  char execPath[2048];
  uint32_t execPathSize = sizeof(execPath);
  _NSGetExecutablePath(&execPath[0], &execPathSize);
  basePath = std::string(execPath);

  while (basePath.size() > 0 && basePath[basePath.size() - 1] != '/') {
    basePath = basePath.substr(0, basePath.size() - 1);
  }

  while (basePath.size() > 0 && (basePath[basePath.size() - 1] == '/' ||
    basePath[basePath.size() - 1] == '.')) {
    basePath = basePath.substr(0, basePath.size() - 1);
  }
  basePath = basePath + '/';
#endif
  return basePath;
}
