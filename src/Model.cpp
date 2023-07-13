/*
 *  Model.cpp
 *
 *  Created on: 2017/07/13
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#include <stdexcept>

#include <memory>

#include "Model.hpp"
#include "Logger.hpp"
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/quaternion.hpp>
#include "GlbFile.hpp"
namespace small3d {

  Model::Model() {
  }

  Model::Model(File &file, const std::string& meshName) {
    file.load(*this, meshName);
  }

  Model::Model(File &&file, const std::string& meshName) {
    file.load(*this, meshName);
  }

  uint64_t Model::getNumPoses() {
    return numPoses;
  }

  glm::mat4 Model::getJointTransform(size_t joint, uint64_t currentPose) {
    
    glm::mat4 parentTransform(1.0f);
    glm::mat4 transform(1.0f);

    size_t idx = 0;
    bool found = false;
    for (auto& j : joints) {
      for (auto c : j.children) {
        if (c == joints[joint].node) {
          found = true;
          break;
        }
      }
      if (found) break;
      ++idx;
    }

    if (found) {
      parentTransform = getJointTransform(idx, currentPose);
    }

    auto pTranslation = currentPose < joints[joint].translationAnimation.size() ? joints[joint].translationAnimation[currentPose] : joints[joint].translation;
    auto pRotation = currentPose < joints[joint].rotationAnimation.size() ? joints[joint].rotationAnimation[currentPose] : joints[joint].rotation;
    auto pScale = currentPose < joints[joint].scaleAnimation.size() ? joints[joint].scaleAnimation[currentPose] : joints[joint].scale;

    transform = glm::translate(glm::mat4(1.0f), pTranslation) *
      glm::toMat4(pRotation) *
      glm::scale(glm::mat4(1.0f), pScale);

    return parentTransform * transform;
  }

  glm::vec3 Model::getOriginalScale() {
    return origScale;
  }

}
