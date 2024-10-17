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
#include "File.hpp"

namespace small3d
{
  /**
   * @class SceneObject
   *
   * @brief An object that appears on the 3D scene. It is made up of a Model,
   *        together with information for positioning, rotation and collision 
   *        detection. Skeletal animation is supported if contained in the Model.
   *        Otherwise, frame based animation can be used if the SceneObject is
   *        constructed with a vector of Models, each of which will be used as
   *        an animation frame.
   */

  class SceneObject
  {
  private:
    bool skeletal;
    bool animating;
    bool repeatAnimation = true;
    int frameDelay;
    uint64_t currentPose;
    int framesWaited;
    uint64_t getNumPoses();
    std::string name;
    glm::mat4x4 transformation = glm::mat4x4(1);
    glm::vec3 rotationXYZ = glm::vec3(0.0f);
    bool rotationByMatrix = false;
    std::vector<std::shared_ptr<Model>> models;
    std::shared_ptr<BoundingBoxSet> boundingBoxSet = std::shared_ptr<BoundingBoxSet>(new BoundingBoxSet());
    void init(const std::string& name, const uint32_t boundingBoxSubdivisions);
  public:

    /** 
     * @brief Model based constructor (skeletal animation)
     * 
     * @param name  The name of the object
     * @param model The Model for which to create the object (lvalue)
     * @param boundingBoxSubdivisions How many times to subdivide the initially created
     *              bounding box, getting more accurate collision detection
     *              at the expense of performance.
     */
    SceneObject(const std::string& name, const Model& model, const uint32_t boundingBoxSubdivisions = 0);

    /**
     * @brief Model based constructor (skeletal animation)
     *
     * @param name  The name of the object
     * @param model The Model for which to create the object (rvalue)
     * @param boundingBoxSubdivisions How many times to subdivide the initially created
     *              bounding box, getting more accurate collision detection
     *              at the expense of performance.
     */
    SceneObject(const std::string& name, const Model&& model, const uint32_t boundingBoxSubdivisions = 0);


    /**
     * @brief Model vector based constructor (animation using frames)
     *
     * @param name  The name of the object
     * @param models The Model for which to create the object (rvalue)
     * @param boundingBoxSubdivisions How many times to subdivide the initially created
     *              bounding box, getting more accurate collision detection
     *              at the expense of performance.
     */
    SceneObject(const std::string& name, const std::vector<std::shared_ptr<Model>>& models, const uint32_t boundingBoxSubdivisions = 0);

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
     * @brief Get the current animation pose
     * @return The current animation pose
     */
    uint64_t getCurrentPose();

    /**
     * @brief Set the current animation
     * @param animationIdx The index of the current animation
     */
    void setAnimation(uint32_t animationIdx);

    /**
     * @brief Get the bounding box set as models (for debug-rendering)
     * @return The bounding box set models
     */
    std::vector<Model> getBoundingBoxSetModels();

    /**
     * @brief Get the bounding box set extremes (min and max coords)
     * @return The bounding box set extremes
     */
    std::vector<BoundingBoxSet::extremes>& getBoundingBoxSetExtremes();

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
     * @brief: Set the rotation of the object. (Overwrites
     *         transformation entered via setTransformation)
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
     * @brief: Set the transformation matrix of the object. (Overwrites
     *         rotation entered via setRotation)
     *    
     * @param rotation The tranformation matrix
     */
    void setTransformation(const glm::mat4x4& rotation);

    /**
     * @brief: Get the orientation of the object
     *
     * @return The orientation of the object
     */
    const glm::vec3 getOrientation() const;

    /**
     * @brief: Get the tranformation matrix of the object
     *
     * @return The tranformation matrix
     */
    const glm::mat4x4& getTransformation() const;

    /**
     * @brief: Get the rotation of the object in x, y, z representation.
     *         This will NOT work if the rotation was set via the
     *         setTransformation function.
     *
     * @return The rotation in x, y, z representation
     */
    const glm::vec3& getRotationXYZ() const;

    /**
     * @brief Start animating the object
     * @param repeat Repeat the animation after it has finished?
     */
    void startAnimating(bool repeat = true);

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
