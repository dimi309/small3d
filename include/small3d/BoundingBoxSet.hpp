/**
 *  @file  BoundingBoxSet.hpp
 *  @brief Bounding boxes for collision detection
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
   * @brief Set of bounding boxes for a SceneObject, calculated based on the 
   *        vertices of a Model.
   */

  class BoundingBoxSet {
  private:

    uint32_t numBoxes = 0;
    void triangulate();
    void calcExtremes();
    void generateBoxesFromExtremes();
    void generateExtremes(std::vector<float>& vertexData, uint32_t subdivisions);
    void generateSubExtremes(std::vector<float>& vertexData);

  public:

    /**
     * @brief Structure to hold the coordinates of the extremes of each box.
     */
    struct extremes {
    
      float minZ = 0.0f, maxZ = 0.0f, minX = 0.0f, maxX = 0.0f, minY = 0.0f, maxY = 0.0f;
      bool tagged = false;
    
    };

    /**
     * @brief The extreme coordinates (max and min) of each box.
     */
    std::vector<extremes> boxExtremes;

    /**
     * @brief Get the number of boxes contained in the set.
     */

    int getNumBoxes() const;

    /**
     * @brief Default constructor
     */
    BoundingBoxSet();

    /**
     * @brief Constructor that creates a box set, constructed based on the vertex 
     *        data that can be found in a Model.
     * @param vertexData  The vertex data. Array of floats to be interpreted as
     *                    an array of 4 component vertex coordinates.
     * @param subdivisions How many times to subdivide the initially one created
     *                     bounding box, getting more accurate collision detection
     *                     at the expense of performance.
     */

    BoundingBoxSet(std::vector<float>& vertexData, uint32_t subdivisions);

    /**
     * @brief Destructor
     */
    ~BoundingBoxSet() = default;

    /**
     * @brief Vertex coordinates
     */

    std::vector<std::vector<float> > vertices;

    /**
     * @brief Faces vertex indexes (rectangles)
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
     * @param thisRotation The rotation transformation of the box set
     * @return True the point is inside a box, False if not.
     */

    bool contains(const glm::vec3 point, const glm::vec3 thisOffset,
      const glm::mat4x4 thisRotation) const;

    /**
     * @brief Check any of the corners of another set of bounding boxes
     *        is inside any of the boxes of this set.
     * @param otherBoxSet   The other box set
     * @param thisOffset    The offset (location) of this box set
     * @param thisRotation  The rotation transformation of this box set
     * @param otherOffset   The offset (location) of the other box set
     * @param otherRotation The rotation transformation of the other box set
     * @return True if a corner of the other bounding box set is contained in
     *         this set, False otherwise.
     */

    bool containsCorners(const BoundingBoxSet otherBoxSet,
      const glm::vec3 thisOffset,
      const glm::mat4x4 thisRotation,
      const glm::vec3 otherOffset,
      const glm::mat4x4 otherRotation) const;

    /**
     * @brief Get the bounding boxes in a set of Models that can be rendered
     * @return The set of bounding boxes as Models
     */
    std::vector<Model> getModels();

  };
}
