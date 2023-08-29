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
#include <cereal/archives/binary.hpp>
#include <sstream>
#include <ostream>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/memory.hpp>
#include <zlib.h>

namespace small3d {

  Model::Model() {
    numPoses.resize(1);
    numPoses[0] = 0;
  }

  Model::Model(File& file, const std::string& meshName) {
    file.load(*this, meshName);
    numPoses.resize(1);
    numPoses[0] = 0;
  }

  Model::Model(File&& file, const std::string& meshName) {
    numPoses.resize(1);
    numPoses[0] = 0;
    file.load(*this, meshName);
  }

  uint64_t Model::getNumPoses() {
    return numPoses[currentAnimation];
  }

  size_t Model::getNumAnimations() {
    return numPoses.size();
  }
  
  void Model::setAnimation(uint32_t animationIdx) {
    if (numPoses.size() <= animationIdx) {
      throw new std::runtime_error("Cannot set animation to " + std::to_string(animationIdx) + 
        ". There are only " + std::to_string(numPoses.size()) + " animations.");
    }
    currentAnimation = animationIdx;
  }

  glm::mat4 Model::getJointTransform(size_t joint, uint32_t animationIdx, uint64_t currentPose, float seconds) {

    float secondsUsed = seconds;

    // If parameter seconds == 0, it looks like
    // this function has been called for the 
    // first time in the stack sequence.
    // Find in one of the animation sequences
    // a seconds value that corresponds to 
    // the current pose.
    if (seconds == 0.0f && joints[joint].animations.size() > 0) {
      for (auto& anim : joints[joint].animations[animationIdx].animationComponents) {
        if (anim.times.size() > currentPose) {
          secondsUsed = anim.times[currentPose];
          break;
        }
      }
    }
 
    // Find the parent node, if it exists
    size_t idx = 0;
    bool parentFound = false;
    for (auto& j : joints) {
      for (auto c : j.children) {
        if (c == joints[joint].node) {
          parentFound = true;
          break;
        }
      }
      if (parentFound) break;
      ++idx;
    }

    // If parent node exists, get the transform
    // of the parent node that corresponds to the seconds
    // value used
    glm::mat4 parentTransform(1.0f);
    if (parentFound) {
      parentTransform = getJointTransform(idx, animationIdx, currentPose, secondsUsed);
    }

    // By default, the joint is in its initial state
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), joints[joint].translation);
    glm::mat4 rotation = glm::toMat4(joints[joint].rotation);
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), joints[joint].scale);

    // First time a translation, rotation or scale frame
    // are being assigned?
    bool firstT = true, firstR = true, firstS = true;
    
    if (joints[joint].animations.size() > 0) {
      for (auto& animationComponent : joints[joint].animations[animationIdx].animationComponents) {
        auto poseUsed = currentPose;
        bool foundPose = false;

        // Find the pose that corresonds to the seconds
        // used
        uint32_t tidx = 0;
        for (auto& time : animationComponent.times) {

          // Using == it has been observed that at least one sample
          // model used for testing gets deformed.
          if (time >= secondsUsed) {
            poseUsed = tidx;
            foundPose = true;
            break;
          }
          ++tidx;
        }

        // Replace initial joint translation, scale and / or rotation
        // if such transformations are found in the pose used.
        if (foundPose) {
          if (animationComponent.translationAnimation.size() > poseUsed) {
            if (firstT) {
              translation = glm::translate(glm::mat4(1.0f), animationComponent.translationAnimation[poseUsed]);
              firstT = false;
            }
            else {
              translation *= glm::translate(glm::mat4(1.0f), animationComponent.translationAnimation[poseUsed]);
            }
          }
          if (animationComponent.rotationAnimation.size() > poseUsed) {
            if (firstR) {
              rotation = glm::toMat4(animationComponent.rotationAnimation[poseUsed]);
              firstR = false;
            }
            else {
              rotation *= glm::toMat4(animationComponent.rotationAnimation[poseUsed]);
            }
          }
          if (animationComponent.scaleAnimation.size() > poseUsed) {
            if (firstS) {
              scale = glm::scale(glm::mat4(1.0f), animationComponent.scaleAnimation[poseUsed]);
              firstS = false;
            }
            else {
              scale *= glm::scale(glm::mat4(1.0f), animationComponent.scaleAnimation[poseUsed]);
            }
          }
        }
      }
    }
    return parentTransform * translation * rotation * scale * joints[joint].transformation;
  }

  glm::vec3 Model::getOriginalScale() {
    return origScale;
  }

  void Model::saveBinary(const std::string binaryFilePath) {

    const uint32_t CHUNK = 16384;

    std::stringstream ss(std::ios::out | std::ios::binary | std::ios::trunc);

    cereal::BinaryOutputArchive oarchive(ss);
    oarchive(*this);

    unsigned char out[CHUNK];
    uint32_t have = 0;
    z_stream strm;
    int flush;

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    if (deflateInit(&strm, Z_DEFAULT_COMPRESSION) != Z_OK) {
      throw new std::runtime_error("Failed to initialise deflate stream.");
    }

    auto strBuffer = ss.str();
    strm.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(strBuffer.c_str()));
    strm.avail_in = strBuffer.length();
    std::string compressedData = "";
    int defRet = 0;
    do {
      strm.avail_out = CHUNK;
      strm.next_out = out;

      defRet = deflate(&strm, strm.avail_in > 0 ? Z_NO_FLUSH : Z_FINISH);
      if (defRet == Z_STREAM_ERROR) {
        LOGERROR("Stream error");
      }
      have = CHUNK - strm.avail_out;
      for (uint32_t idx = 0; idx < have; ++idx) {
        compressedData += out[idx];
      }
    } while (defRet != Z_STREAM_END);
    deflateEnd(&strm);

    std::ofstream ofstr(binaryFilePath, std::ios::out | std::ios::binary);
    ofstr.write(compressedData.c_str(), compressedData.length());
    ofstr.close();
  }

}
