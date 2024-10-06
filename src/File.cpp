/**
 *  File.cpp
 *
 *  Created on: 2021/02/25
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#include "File.hpp"
#include "BasePath.hpp"

namespace small3d {
  File::File(const std::string& fileLocation) {
    this->fullPath = fileLocation[0] == '/' ? fileLocation : getBasePath() + fileLocation;
  }
}
