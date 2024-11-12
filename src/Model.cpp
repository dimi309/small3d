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
#include "GlbFile.hpp"
#include <cereal/archives/binary.hpp>
#include <sstream>
#include <ostream>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/memory.hpp>
#include <zlib.h>
#include <algorithm>

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

  Mat4 Model::getTransform(uint32_t animationIdx, uint64_t currentPose, float seconds) {
    float secondsUsed = seconds;

    // If parameter seconds == 0, it looks like
    // this function has been called for the 
    // first time in the stack sequence.
    // Find in one of the animation sequences
    // a seconds value that corresponds to 
    // the current pose.
    if (seconds == 0.0f && animations.size() > 0) {

      if (auto animFound = std::find_if(animations[animationIdx].animationComponents.begin(),
        animations[animationIdx].animationComponents.end(),
        [&currentPose](const auto& anim) {return anim.times.size() > currentPose; });
        animFound != std::end(animations[animationIdx].animationComponents)) {
        secondsUsed = (*animFound).times[currentPose];
      }
    }

    // By default, the joint is in its initial state
    Mat4 translation(1.0f);
    Mat4 rotation(1.0f);
    Mat4 scale(1.0f);

    if (animations.size() > 0) {

      // First time a translation, rotation or scale frame
      // are being assigned?
      bool firstT = true, firstR = true, firstS = true;

      for (const auto& animationComponent : animations[animationIdx].animationComponents) {
        auto poseUsed = currentPose;
        bool foundPose = false;

        // Find the pose that corresonds to the seconds
        // used
        uint32_t tidx = 0;
        for (const auto& time : animationComponent.times) {

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
              translation = translate(Mat4(1.0f), animationComponent.translationAnimation[poseUsed]);
              firstT = false;
            }
            else {
              translation *= translate(Mat4(1.0f), animationComponent.translationAnimation[poseUsed]);
            }
          }
          if (animationComponent.rotationAnimation.size() > poseUsed) {
            if (firstR) {

              rotation = animationComponent.rotationAnimation[poseUsed].toMatrix();

              firstR = false;
            }
            else {
              rotation *= animationComponent.rotationAnimation[poseUsed].toMatrix();

            }
          }
          if (animationComponent.scaleAnimation.size() > poseUsed) {
            if (firstS) {
              scale = small3d::scale(Mat4(1.0f), animationComponent.scaleAnimation[poseUsed]);
              firstS = false;
            }
            else {
              scale *= small3d::scale(Mat4(1.0f), animationComponent.scaleAnimation[poseUsed]);
            }
          }
        }
      }
    }
    return translation * rotation * scale;

  }

  Mat4 Model::getJointTransform(size_t joint, uint32_t animationIdx, uint64_t currentPose, float seconds) {

    float secondsUsed = seconds;

    // If parameter seconds == 0, it looks like
    // this function has been called for the 
    // first time in the stack sequence.
    // Find in one of the animation sequences
    // a seconds value that corresponds to 
    // the current pose.
    if (seconds == 0.0f && joints[joint].animations.size() > 0) {

      if (auto animFound = std::find_if(joints[joint].animations[animationIdx].animationComponents.begin(),
        joints[joint].animations[animationIdx].animationComponents.end(),
        [&currentPose](const auto& anim) {return anim.times.size() > currentPose; });
        animFound != std::end(joints[joint].animations[animationIdx].animationComponents)) {
        secondsUsed = (*animFound).times[currentPose];
      }
    }

    // Find the parent node, if it exists
    size_t idx = 0;
    bool parentFound = false;
    for (const auto& j : joints) {

      auto jointnode = joints[joint].node;

      if (std::any_of(j.children.begin(), j.children.end(), [&jointnode](const auto& c) {
        return c == jointnode;
        })) {
        parentFound = true;
        break;
      }

      ++idx;
    }

    // If parent node exists, get the transform
    // of the parent node that corresponds to the seconds
    // value used
    Mat4 parentTransform(1.0f);
    if (parentFound) {
      parentTransform = getJointTransform(idx, animationIdx, currentPose, secondsUsed);
    }

    // By default, the joint is in its initial state
    Mat4 translation = translate(Mat4(1.0f), joints[joint].translation);

    auto rotation = joints[joint].rotation.toMatrix();


    Mat4 scale = small3d::scale(Mat4(1.0f), joints[joint].scale);


    if (joints[joint].animations.size() > 0) {

      // First time a translation, rotation or scale frame
      // are being assigned?
      bool firstT = true, firstR = true, firstS = true;

      for (const auto& animationComponent : joints[joint].animations[animationIdx].animationComponents) {
        auto poseUsed = currentPose;
        bool foundPose = false;

        // Find the pose that corresonds to the seconds
        // used
        uint32_t tidx = 0;
        for (const auto& time : animationComponent.times) {

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
              translation = translate(Mat4(1.0f), animationComponent.translationAnimation[poseUsed]);
              firstT = false;
            }
            else {
              translation *= translate(Mat4(1.0f), animationComponent.translationAnimation[poseUsed]);
            }
          }
          if (animationComponent.rotationAnimation.size() > poseUsed) {
            if (firstR) {
              rotation = animationComponent.rotationAnimation[poseUsed].toMatrix();
              firstR = false;
            }
            else {
              rotation *= animationComponent.rotationAnimation[poseUsed].toMatrix();
            }
          }
          if (animationComponent.scaleAnimation.size() > poseUsed) {
            if (firstS) {
              scale = small3d::scale(Mat4(1.0f), animationComponent.scaleAnimation[poseUsed]);
              firstS = false;
            }
            else {
              scale *= small3d::scale(Mat4(1.0f), animationComponent.scaleAnimation[poseUsed]);
            }
          }
        }
      }
    }
    return  parentTransform * translation * rotation * scale * joints[joint].transformation;
  }

  Vec3 Model::getOriginalScale() {
    return origScale;
  }

  void Model::saveBinary(const std::string& binaryFilePath) {

    const uint32_t CHUNK = 16384;

    std::stringstream ss(std::ios::out | std::ios::binary | std::ios::trunc);

    cereal::BinaryOutputArchive oarchive(ss);
    oarchive(*this);

    unsigned char out[CHUNK];
    z_stream strm;

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
      uint32_t have = CHUNK - strm.avail_out;
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
