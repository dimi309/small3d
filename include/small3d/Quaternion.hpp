/**
 * @file Quaternion.hpp
 * @brief Quaternion struct
 *
 * Created on: 2024/10/20
 *     Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */
#pragma once

#include <glm/glm.hpp>

namespace small3d {

  struct Quaternion {

    float x;
    float y;
    float z;
    float w;

    glm::mat4 toMatrix() const;

    template <class Archive>
    void serialize(Archive& archive) {
      archive(w, x, y, z);
    }

    Quaternion operator*(const Quaternion& other);
    

  };

}

