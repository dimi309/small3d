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
   * @brief Set of bounding boxes for a SceneObject, normally loaded from a
   *        Wavefront file, allowing for collision detection.
   *        In order to create these in Blender for example (see blender.org),
   *        just place them in the preferred position over a model. Ideally,
   *        they should be aligned with the axes, (but note that small3d does
   *        more than just a simple axis-aligned bounding box collision
   *        detection).
   *
   *        Export the bounding boxes to a Wavefront file separately from the
   *        model. You can do this if you "save as" a new file after placing
   *        the boxes and deleting the original model. During export, only set
   *        the options Apply Modifiers, Include Edges (but not in newer
   *        versions of Blender, where it is not available),
   *        Objects as OBJ Objects and Keep Vertex Order. On the contrary to
   *        what is the case when exporting the Model itself, more than one
   *        bounding box objects can be exported to the same Wavefront file.
   *
   *        It is good to keep the default origin in Blender for the models
   *        as well as the bounding boxes. User-set origins are ignored by
   *        Blender when exporting Wavefront files. That can cause
   *        misalignments between bounding boxes and models, even if
   *        the origins of both have been properly set to a new position.
   */

  class BoundingBoxSet {
  private:

    int numBoxes;
    void loadFromFile(std::string fileLocation);
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
     * @brief Constructor that loads a set of bounding boxes from a Wavefront file
     * @param fileLocation Location of the Wavefront file containing the
     *        bounding boxes
     *
     */

    BoundingBoxSet(const std::string& fileLocation = "");

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
