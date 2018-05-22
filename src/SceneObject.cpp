/*
 *  SceneObject.cpp
 *
 *  Created on: 2014/10/19
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#include "SceneObject.hpp"

namespace small3d {

  SceneObject::SceneObject(const std::string name, const std::string modelPath,
			   const int numFrames,
			   const std::string boundingBoxSetPath) :
    offset(0,0,0), rotation(0,0,0), boundingBoxSet(boundingBoxSetPath) {
    
    initLogger();
    this->name = name;
    animating = false;
    framesWaited = 0;
    frameDelay = 1;
    currentFrame = 0;
    this->numFrames = numFrames;

    if (numFrames > 1) {
      LOGINFO("Loading " + name + " animated model (this may take a while):");
      for (int idx = 0; idx < numFrames; ++idx) {
        std::stringstream lss;
        lss << "Frame " << idx + 1 << " of " << numFrames << "...";
        LOGINFO(lss.str());
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(6) << idx + 1;
        std::string frameNum = ss.str();
        Model model1(modelPath + "_" + frameNum + ".obj");
        model.push_back(model1);
      }
    }
    else {
      Model model1(modelPath);
      model.push_back(model1);
    }
  }

  Model& SceneObject::getModel() {
    return model[currentFrame];
  }

  const std::string SceneObject::getName() const {
    return name;
  }

  void SceneObject::startAnimating() {
    animating = true;
  }

  void SceneObject::stopAnimating() {
    animating = false;
  }

  void SceneObject::resetAnimation() {
    currentFrame = 0;
  }

  void SceneObject::setFrameDelay(const int delay) {
    this->frameDelay = delay;
  }

  void SceneObject::animate() {
    if (animating) {
      ++framesWaited;
      if (framesWaited == frameDelay) {
        framesWaited = 0;
        ++currentFrame;
        if (currentFrame == numFrames) {
          currentFrame = 0;
        }
      }
    }
  }

  bool SceneObject::collidesWith(const glm::vec3 point) const {
    if (boundingBoxSet.vertices.size() == 0) {
      throw std::runtime_error("No bounding boxes have been provided for " +
			       name +
			       ", so collision detection is not enabled.");
    }
    return boundingBoxSet.collidesWith(point, this->offset, this->rotation);
  }

  bool SceneObject::collidesWith(SceneObject otherObject) const {
    if (boundingBoxSet.vertices.size() == 0) {
      throw std::runtime_error("No bounding boxes have been provided for " +
			       name +
			       ", so collision detection is not enabled.");
    }

    if (otherObject.boundingBoxSet.vertices.size() == 0) {
      throw std::runtime_error("No bounding boxes have been provided for " +
			       otherObject.name +
			       ", so collision detection is not enabled.");
    }

    // Checking whether the boxes of this object are within the boxes of the
    // other object or vice versa
    return boundingBoxSet.collidesWith(otherObject.boundingBoxSet, this->offset,
				       this->rotation, otherObject.offset,
				       otherObject.rotation) ||
      otherObject.boundingBoxSet.collidesWith(boundingBoxSet, otherObject.offset,
					      otherObject.rotation,
					      this->offset, this->rotation);
  }

  bool SceneObject::isAnimated() const {
    return numFrames > 1;
  }

}
