/**
 * @file  SceneObject.hpp
 * @brief Header of the SceneObject class
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
   * @brief An object that appears on the 3D scene. It is made up of one or more
   *        models (the latter for animation), together with information about
   *        positioning and rotation and collision detection functionality.
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

  public:
    /**
     * @brief The models making up the object's animation. Do not manipulate
     *        directly.
     */
    std::vector<Model> models;

    /**
     * @brief Constructor
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
     *
     * @param numFrames The number of frames, if the object is animated. A
     *                  single animation sequence is supported per object and
     *                  the first frame is considered to be the non-moving 
     *                  state.
     *
     * @param boundingBoxSetPath The path to the file containing the object's
     *                           bounding box set. If no such path is given, the
     *                           object cannot be checked for collision
     *                           detection.
     * @param startFrameIndex The index number in the filename of the first file
     *                        of the animation sequence. The default value is 1.
     *                        If not loading an animation sequence, this parameter
     *                        is ignored.
     */
    SceneObject(const std::string name, const std::string modelPath,
		const int numFrames = 1,
		const std::string boundingBoxSetPath = "", const int startFrameIndex = 1);

    /**
     * @brief Destructor
     */
    ~SceneObject() = default;

    /**
     * @brief Get the object's model
     * @return The object's model
     */
    Model& getModel() ;

    /**
     * @brief Is this an animated or a static object (is it associated with more than
     *        one frames/models)?
     * @return True if animated, False otherwise.
     */
    bool isAnimated() const;

    /**
     * @brief Get the name of the object
     * @return The name of the object
     */
    const std::string getName() const;

    /**
     * Offset (position) of the object
     */
    glm::vec3 offset;

    /**
     * Rotation of the object (on x, y, z axes respectively)
     */
    glm::vec3 rotation;

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
     * @brief The bounding boxes for the object, used for collision detection.
     */

    BoundingBoxSet boundingBoxSet;

    /**
     * @brief Check if the object collides with a given point
     * @param  point The point
     * @return True if a collision is detected, False otherwise.
     */

    bool collidesWith(const glm::vec3 point) const;

    /**
     *
     * @brief	Check if the object collides with another given object.
     * @param	otherObject The other object.
     * @return	True if there is a collision, False if not.
     */

    bool collidesWith(SceneObject otherObject) const;

  };
  
}
