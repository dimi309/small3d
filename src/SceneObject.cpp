/*
 *  SceneObject.cpp
 *
 *  Created on: 2014/10/19
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#include "SceneObject.hpp"
#include "WavefrontFile.hpp"
#include "GlbFile.hpp"
#include <exception>

namespace small3d {

  SceneObject::SceneObject(const std::string name, const std::string modelPath,
    const std::string& modelMeshName, const uint32_t boundingBoxSubdivisions) {
    initLogger();
    this->name = name;
    animating = false;
    framesWaited = 0;
    frameDelay = 1;
    currentFrame = 0;
    this->numFrames = 1;
    LOGDEBUG("Trying to load " + modelPath + " as glTF.");
    try {
      Model model1(GlbFile(modelPath), modelMeshName);
      models.push_back(model1);
      boundingBoxSet = BoundingBoxSet(model1.vertexData, boundingBoxSubdivisions);
    }
    catch (std::runtime_error& e) {
      if (std::string(e.what()).find(GlbFile::NOTGLTF)) {
        LOGDEBUG("Trying to load " + modelPath + " as Wavefront.");
        try {
          Model model1(WavefrontFile(modelPath), modelMeshName);
          models.push_back(model1);
          boundingBoxSet = BoundingBoxSet(model1.vertexData, boundingBoxSubdivisions);
        }
        catch (std::runtime_error& e2) {
          LOGERROR(e2.what());
          throw std::runtime_error("Failed to open " + modelPath + " both as .glb file and as .obj file.");
        }
      }
      else throw e;
    }
  }

  SceneObject::SceneObject(const std::string name, const std::string modelPath,
    const int numFrames,
    const int startFrameIndex,
    const uint32_t boundingBoxSubdivisions) :
    offset(0, 0, 0), rotation(0, 0, 0) {

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
        Model model1(WavefrontFile(modelPath + "_" + frameNum + ".obj"), "");
        models.push_back(model1);
      }
    }
    else {
      WavefrontFile w(modelPath);
      Model model1(w, "");
      models.push_back(model1);
    }
    boundingBoxSet = BoundingBoxSet(getModel().vertexData, boundingBoxSubdivisions);
  }

  Model& SceneObject::getModel() {
    return models[currentFrame];
  }

  const std::string SceneObject::getName() const {
    return name;
  }

  void SceneObject::startAnimating() {
    animating = true;
    framesWaited = 0;
  }

  void SceneObject::stopAnimating() {
    animating = false;
    framesWaited = 0;
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
        if (wavefront) {
          ++currentFrame;
          if (currentFrame == numFrames) {
            currentFrame = 0;
          }
        }
        else {
          models[0].animate();
        }
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
