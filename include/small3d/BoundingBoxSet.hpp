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
#include "Model.hpp"

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
    void triangulate();
    void calcExtremes();

  public:

    /**
     * @brief Structure to hold the coordinates of the extremes of each box.
     */
    typedef struct extremes_ {
      float minZ, maxZ, minX, maxX, minY, maxY;
    } extremes;

    /**
     * @brief The extreme coordinates (max and min) of each box.
     */
    std::vector<extremes> boxExtremes;

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
     * @brief Faces vertex indexes read from Wavefront .obj file (rectangles)
     */

    std::vector<std::vector<unsigned int> > facesVertexIndexes;

    /**
    * @brief Faces vertex indexes (triangles, for rendering)
    */
    std::vector<std::vector<unsigned int> > facesVertexIndexesTriangulated;

    /**
     * @brief Check if a point is inside any of the boxes.
     * @param point        The point (as a vector)
     * @param thisOffset   The offset (location) of the box set
     * @param thisRotation The rotation of the box set
     * @return True the point is inside a box, False if not.
     */

    bool contains(const glm::vec3 point, const glm::vec3 thisOffset,
      const glm::vec3 thisRotation) const;

    /**
     * @brief Check any of the corners of another set of bounding boxes 
     *        is inside any of the boxes of this set.
     * @param otherBoxSet   The other box set
     * @param thisOffset    The offset (location) of this box set
     * @param thisRotation  The rotation of this box set
     * @param otherOffset   The offset (location) of the other box set
     * @param otherRotation The rotation of the other box set
     * @return True if a corner of the other bounding box set is contained in 
     *         this set, False otherwise.
     */

    bool containsCorners(const BoundingBoxSet otherBoxSet,
      const glm::vec3 thisOffset,
      const glm::vec3 thisRotation,
      const glm::vec3 otherOffset,
      const glm::vec3 otherRotation) const;

    /**
     * @brief Get the bounding boxes in a set of Models that can be rendered
     * @return The set of bounding boxes as Models
     */
    std::vector<Model> getModels();

  };
}
