/**
 *  @file  BoundingBoxSet.hpp
 *  @brief Header of the BoundingBoxSet class.
 *
 *  Created on: 2014/10/19
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 *
 */

#pragma once

#include <memory>
#include "Logger.hpp"
#include <vector>
#include <glm/glm.hpp>

namespace small3d {

  /**
   * @class BoundingBoxSet
   * @brief Set of bounding boxes for a SceneObject, loaded from a Wavefront
   * file, allowing for collision detection (see README.md).
   */

  class BoundingBoxSet {
  private:
    int numBoxes;
    void loadFromFile(std::string fileLocation);
    
  public:

    /**
     * @brief Get the number of boxes contained in the set.
     */
    
    int getNumBoxes() const;

    /**
     * @brief Constructor
     * @param fileLocation Location of the Wavefront file containing the
     *        bounding boxes
     *
     */

    BoundingBoxSet(const std::string fileLocation = "");

    /**
     * @brief Destructor
     */
    ~BoundingBoxSet() = default;

    /**
     * @brief Vertex coordinates read from Wavefront .obj file
     */

    std::vector<std::vector<float> > vertices;

    /**
     * @brief Faces vertex indexes read from Wavefront .obj file
     */

    std::vector<std::vector<unsigned int> > facesVertexIndexes;

    /**
     * @brief Check if a point collides (or is inside) any of the boxes of the
     *        box set, assuming that they are in a given offset and have a
     *        certain rotation.
     * @param point        The point (as a vector)
     * @param thisOffset   The offset (location) of the box set
     * @param thisRotation The rotation of the box set
     * @return True if there is a collision, False if not.
     */

    bool collidesWith(const glm::vec3 point, const glm::vec3 thisOffset,
      const glm::vec3 thisRotation) const;

    /**
     * @brief Check if another set of bounding boxes is located with this set
     *        (even partially), thus colliding with it.
     * @param otherBoxSet   The other box set
     * @param thisOffset    The offset (location) of this box set
     * @param thisRotation  The rotation of this box set
     * @param otherOffset   The offset (location) of the other box set
     * @param otherRotation The rotation of the other box set
     * @return True if there is a collision, False if not.
     */

    bool collidesWith(const BoundingBoxSet otherBoxSet,
		      const glm::vec3 thisOffset,
		      const glm::vec3 thisRotation,
		      const glm::vec3 otherOffset,
		      const glm::vec3 otherRotation) const;

  };
}
