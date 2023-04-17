/*
 *  SceneObject.cpp
 *
 *  Created on: 2014/10/19
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#include "SceneObject.hpp"
#include "GlbFile.hpp"
#include <exception>

namespace small3d {

  SceneObject::SceneObject(const std::string& name, const std::string& modelPath,
    const std::string& modelMeshName, const uint32_t boundingBoxSubdivisions) {
    initLogger();
    this->name = name;
    animating = false;
    framesWaited = 0;
    frameDelay = 1;
    currentFrame = 0;
    this->numFrames = 1;
    LOGDEBUG("Trying to load " + modelPath + " as glTF.");
    model = std::make_shared<Model>(GlbFile(modelPath), modelMeshName);
    boundingBoxSet = std::make_shared<BoundingBoxSet>(model->vertexData, model->getOriginalScale(), boundingBoxSubdivisions);
  }

  SceneObject::SceneObject(const std::string& name, const Model& model, const uint32_t boundingBoxSubdivisions) {
    initLogger();
    this->name = name;
    animating = false;
    framesWaited = 0;
    frameDelay = 1;
    currentFrame = 0;
    this->numFrames = 1;
    this->model = std::make_shared<Model>(model);
    boundingBoxSet = std::make_shared<BoundingBoxSet>(this->model->vertexData, this->model->getOriginalScale(), boundingBoxSubdivisions);
  }

  Model& SceneObject::getModel() {
    return *model;
  }

  std::vector<Model> SceneObject::getBoundingBoxSetModels() {
    return boundingBoxSet->getModels();
  }

  std::vector<BoundingBoxSet::extremes>& SceneObject::getBoundingBoxSetExtremes() {
    return boundingBoxSet->boxExtremes;
  }

  const std::string SceneObject::getName() const {
    return name;
  }

  void SceneObject::setRotation(const glm::vec3& rotation) {
    rotationByMatrix = false;
    this->rotationXYZ = rotation;
    this->transformation = glm::rotate(glm::mat4x4(1.0f), rotation.y, glm::vec3(0.0f, 1.0f, 0.0f)) *
      glm::rotate(glm::mat4x4(1.0f), rotation.x, glm::vec3(1.0f, 0.0f, 0.0f)) *
      glm::rotate(glm::mat4x4(1.0f), rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
  }

  void SceneObject::rotate(const glm::vec3& rotation) {
    if (rotationByMatrix) {
      throw std::runtime_error("Attempted x, y, z representation rotation, while having set the initial rotation by matrix.");
    }
    else {
      this->rotationXYZ += rotation;
      this->transformation = glm::rotate(glm::mat4x4(1.0f), this->rotationXYZ.y, glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::rotate(glm::mat4x4(1.0f), this->rotationXYZ.x, glm::vec3(1.0f, 0.0f, 0.0f)) *
        glm::rotate(glm::mat4x4(1.0f), this->rotationXYZ.z, glm::vec3(0.0f, 0.0f, 1.0f));
    }
  }

  void SceneObject::setTransformation(const glm::mat4x4& rotation) {
    this->transformation = rotation;
    rotationByMatrix = true;
    this->rotationXYZ = glm::vec3(0.0f);
  }

  const glm::vec3 SceneObject::getOrientation() const {
    auto orientationVec4 = this->transformation * glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);
    return glm::vec3(orientationVec4.x, orientationVec4.y, orientationVec4.z);
  }

  const glm::mat4x4& SceneObject::getTransformation() const {
    return this->transformation;
  }

  const glm::vec3& SceneObject::getRotationXYZ() const {
    if (rotationByMatrix) {
      throw std::runtime_error("Attempted x, y, z representation rotation retrieval. This cannot be done if the setTransformation function has been used on the object.");
    }
    return this->rotationXYZ;
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
          model->animate();
        }
      }
    }
  }

  bool SceneObject::contains(const glm::vec3& point) const {
    if (boundingBoxSet->vertices.size() == 0) {
      throw std::runtime_error("No bounding boxes have been provided for " +
        name +
        ", so collision detection is not enabled.");
    }
    return boundingBoxSet->contains(point, this->position, this->getTransformation());
  }

  bool SceneObject::containsCorners(const SceneObject& otherObject) const {
    if (boundingBoxSet->vertices.size() == 0) {
      throw std::runtime_error("No bounding boxes have been provided for " +
        name +
        ", so collision detection is not enabled.");
    }

    if (otherObject.boundingBoxSet->vertices.size() == 0) {
      throw std::runtime_error("No bounding boxes have been provided for " +
        otherObject.name +
        ", so collision detection is not enabled.");
    }

    return boundingBoxSet->containsCorners(*otherObject.boundingBoxSet, this->position,
      this->transformation, otherObject.position,
      otherObject.transformation);
  }

}
