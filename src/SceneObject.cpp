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
			   const std::string boundingBoxSetPath,
         const int startFrameIndex,
         const uint32_t boundingBoxSubdivisions) :
    offset(0,0,0), rotation(0,0,0) {

    wavefront = true;
    
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
        ss << std::setfill('0') << std::setw(6) << idx + startFrameIndex;
        std::string frameNum = ss.str();
        Model model1(modelPath + "_" + frameNum + ".obj");
        models.push_back(model1);
      }
    }
    else {
      Model model1(modelPath);
      models.push_back(model1);
    }

    if (boundingBoxSetPath != "") {
      boundingBoxSet = BoundingBoxSet(boundingBoxSetPath);
    }
    else {
      boundingBoxSet = BoundingBoxSet(getModel().vertexData, boundingBoxSubdivisions);
    }

  }

  SceneObject::SceneObject(const std::string name, const std::string modelPath,
    const std::string& modelMeshName, const std::string& modelArmatureName,
    const std::string& modelAnimationName,
    const uint32_t boundingBoxSubdivisions) {
    initLogger();
    this->name = name;
    animating = false;
    framesWaited = 0;
    frameDelay = 1;
    currentFrame = 0;
    this->numFrames = 1;
    Model model1(modelPath, modelMeshName, modelArmatureName, modelAnimationName);
    models.push_back(model1);
    boundingBoxSet = BoundingBoxSet(model1.vertexData, boundingBoxSubdivisions);
  }

  Model& SceneObject::getModel() {
    return models[currentFrame];
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
      if (wavefront) {
        ++framesWaited;
        if (framesWaited == frameDelay) {
          framesWaited = 0;
          ++currentFrame;
          if (currentFrame == numFrames) {
            currentFrame = 0;
          }
        }
      }
      else {
        models[0].animate();
      }
    }
  }

  bool SceneObject::contains(const glm::vec3 point) const {
    if (boundingBoxSet.vertices.size() == 0) {
      throw std::runtime_error("No bounding boxes have been provided for " +
			       name +
			       ", so collision detection is not enabled.");
    }
    return boundingBoxSet.contains(point, this->offset, this->rotation);
  }

  bool SceneObject::containsCorners(SceneObject otherObject) const {
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

    return boundingBoxSet.containsCorners(otherObject.boundingBoxSet, this->offset,
				       this->rotation, otherObject.offset,
				       otherObject.rotation);
  }

}
