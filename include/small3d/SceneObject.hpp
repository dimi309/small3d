/**
 * @file  SceneObject.hpp
 * @brief An object on the small3d scene
 *
 *  Created on: 2014/10/19
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#pragma once

#include <vector>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <memory>

#include "Model.hpp"
#include "Logger.hpp"
#include "Image.hpp"
#include "BoundingBoxSet.hpp"
#include <glm/glm.hpp>

namespace small3d
{
  /**
   * @class SceneObject
   *
   * @brief An object that appears on the 3D scene. It is made up of a Model,
   *        together with information for positioning, rotation and collision 
   *        detection. Skeletal animation is supported for models loaded from
   *        glTF files. A constructor that loads multiple Wavefront files to 
   *        construct a frame-based animation sequence is also provided.
   *
   */

  class SceneObject
  {
  private:

    bool animating;
    int frameDelay;
    int currentFrame;
    int framesWaited;
    int numFrames;
    std::string name;
    bool wavefront = false;
    glm::mat4x4 rotation = glm::mat4x4(1);
    glm::vec3 rotationXYZ = glm::vec3(0.0f);
    bool rotationByMatrix = false;
    std::shared_ptr<std::vector<Model>> models = std::shared_ptr<std::vector<Model>>(new std::vector<Model>());
    std::shared_ptr<BoundingBoxSet> boundingBoxSet = std::shared_ptr<BoundingBoxSet>(new BoundingBoxSet());
  public:

    /**
     * @brief File-loading constructor, supporting Wavefront and glTF .glb files.
     *
     * @param name      The name of the object 
     * @param modelPath The path to the file
     *
     * @param modelMeshName The name of the mesh / object in the file which will be loaded
     *                      as the model ("" to load the first object found).
     * @param boundingBoxSubdivisions How many times to subdivide the initially created
     *                        bounding box, getting more accurate collision detection
     *                        at the expense of performance.
     */
    SceneObject(const std::string& name, const std::string& modelPath,
      const std::string& modelMeshName, const uint32_t boundingBoxSubdivisions = 0);

    /**
     * @brief Wavefront-only (.obj) loading constructor that also supports loading
     *        frame-based animation.
     *
     * @param name      The name of the object
     * @param modelPath The path to the file containing the object's model.
     *                  When animating (also see numFrames parameter) it is
     *                  assumed that multiple files will be loaded, the name
     *                  of which contains a 6 digit suffix (after an
     *                  underscore) representing the animation sequence, and
     *                  terminates with the .obj extension. This is also the
     *                  format used by Blender when exporting animations to
     *                  Wavefront files. When loading multiple models for
     *                  animation do not enter a full filename. Skip the
     *                  suffix and the extension. For example an animation
     *                  sequence made up of files named frog_000001.obj,
     *                  frog_000002.obj etc. should be loaded using the
     *                  parameter "directory/frog" here.
     *                  For exporting animations from Blender to a series of 
     *                  Wavefront files in a way that will allow this constructor
     *                  class to load them, select "Animation" and 
     *                  "Apply Modifiers" when exporting to Wavefront.
     *
     * @param numFrames The number of frames, if the object is animated. A
     *                  single animation sequence is supported per object and
     *                  the first frame is considered to be the non-moving
     *                  state.
     *
     * @param startFrameIndex The index number in the filename of the first file
     *                        of the animation sequence. The default value is 1.
     *                        If not loading an animation sequence, this parameter
     *                        is ignored.
     * @param boundingBoxSubdivisions How many times to subdivide the initially created
     *                        bounding box, getting more accurate collision detection
     *                        at the expense of performance.
     */
    SceneObject(const std::string& name, const std::string& modelPath,
      const int numFrames = 1, const int startFrameIndex = 1, 
      const uint32_t boundingBoxSubdivisions = 0);

    /**
     * @brief Destructor
     */
    ~SceneObject() = default;

    /**
     * @brief Get the object's model
     * @return The object's model
     */
    Model& getModel();


    /**
     * @brief Get the bounding box set as models (for debug-rendering)
     * @return The bounding box set models
     */
    std::vector<Model> getBoundingBoxSetModels();

    /**
     * @brief Get the bounding box set extremes (min and max coords)
     * @return The bounding box set extremes
     */
    std::vector<BoundingBoxSet::extremes> getBoundingBoxSetExtremes();

    /**
     * @brief Get the name of the object
     * @return The name of the object
     */
    const std::string getName() const;

    /**
     * Position of the object
     */
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    
    /**
     * @brief: Set the rotation of the object 
     * 
     * @param rotation The rotation (x, y, z)
     */
    void setRotation(const glm::vec3& rotation);

    /**
     * @brief: Modify the rotation of the object
     *
     * @param rotation The rotation to modify by (x, y, z)
     */
    void rotate(const glm::vec3& rotation);

    /**
     * @brief: Set the rotation of the object
     *         by transformation matrix
     * 
     * @param rotation The rotation tranformation matrix
     */
    void setRotation(const glm::mat4x4& rotation);

    /**
     * @brief: Get the orientation of the object
     *
     * @return The orientation of the object
     */
    const glm::vec3 getOrientation() const;

    /**
     * @brief: Get the rotation of the object
     *         by transformation matrix
     *
     * @return The rotation tranformation matrix
     */
    const glm::mat4x4 getRotation() const;

    /**
     * @brief: Get the rotation of the object in x, y, z representation.
     *         This will NOT work if the rotation was set via the
     *         setRotation(mat4x4) function.
     *
     * @return The rotation in x, y, z representation
     */
    const glm::vec3 getRotationXYZ() const;

    /**
     * @brief Start animating the object
     */
    void startAnimating();

    /**
     * @brief Stop animating the object
     */
    void stopAnimating();

    /**
     * @brief Reset the animation sequence (go to the first frame)
     */
    void resetAnimation();

    /**
     * @brief Set the animation speed
     * @param delay The delay between each animation frame, expressed in number
     *              of game frames
     */
    void setFrameDelay(const int delay);

    /**
     * @brief Process animation (progress current frame if necessary)
     */
    void animate();

    /**
     * @brief  Check if the bounding boxes of this object contain
     *         a given point.
     * @param  point The point
     * @return True if the point is contained in the bounding boxes
     *         of the object, False otherwise.
     */

    bool contains(const glm::vec3& point) const;

    /**
     *
     * @brief	 Check if the bounding boxes of this object contain
     *         a corner of the bounding boxes of another object.
     * @param	 otherObject The other object.
     * @return True if the bounding boxes of this object contain
     *         a corner of the bounding boxes of another object,
     *         False otherwise.
     */

    bool containsCorners(const SceneObject& otherObject) const;

    friend class Renderer;

  };

}
