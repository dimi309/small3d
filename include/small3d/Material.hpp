/**
 *  @file Material.hpp
 *  @brief Material structure
 *
 *  Created on: 2023/09/19
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#pragma once

#include "Math.hpp"

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
    Vec3 ambientColour = Vec3(0.5f);
    Vec3 diffuseColour = Vec3(0.0f);
    Vec3 specularColour = Vec3(0.0f);
    Vec3 emissiveCoefficient = Vec3(0.0f);
    /**
     * @brief Optical density / index of refraction
     *
     */
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
