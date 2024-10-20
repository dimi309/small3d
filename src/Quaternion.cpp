/**
 *  Quaternion.cpp
 *
 *  Created on: 2024/10/20
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#include "Quaternion.hpp"

namespace small3d {

  glm::mat4 Quaternion::toMatrix() const {

    glm::mat4 matrix = { 1.0f - 2 * y * y - 2 * z * z, 2 * x * y + 2 * w * z, 2 * x * z - 2 * w * y, 0.0f,
            2 * x * y - 2 * w * z, 1.0f - 2 * x * x - 2 * z * z, 2 * y * z + 2 * w * x, 0.0f,
            2 * x * z + 2 * w * y, 2 * y * z - 2 * w * x, 1.0f - 2 * x * x - 2 * y * y, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f };

    return matrix;

  }

  Quaternion Quaternion::operator*(const Quaternion& other) {
    Quaternion tmp;
    tmp.w = w * other.w;
    tmp.x = x * other.x;
    tmp.y = y * other.y;
    tmp.z = z * other.z;

    return tmp;
  }


}