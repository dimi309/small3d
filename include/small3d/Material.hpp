/**
 *  @file Material.hpp
 *  @brief Material structure
 *
 *  Created on: 2023/09/19
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#pragma once

#include <glm/glm.hpp>

namespace small3d {

  /**
   * @class	Material
   *
   * @brief	A 3D model material
   *
   */

  class Material {

  public:

    float specularExponent = 0.0f;
    glm::vec3 ambientColour = glm::vec3(0.5f);
    glm::vec3 diffuseColour = glm::vec3(0.0f);
    glm::vec3 specularColour = glm::vec3(0.0f);
    glm::vec3 emissiveCoefficient = glm::vec3(0.0f);
    float optDensIndexRef = 0.0f;
    float alpha = 1.0f;

    /**
     * @brief Default constructor
     *
     */
    Material();

    template <class Archive>
    void serialize(Archive& archive) {
      archive(specularExponent, ambientColour, diffuseColour, specularColour, emissiveCoefficient, optDensIndexRef, alpha);
    }

  };
}