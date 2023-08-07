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
  }

  Model::Model(File& file, const std::string& meshName) {
    file.load(*this, meshName);
  }

  Model::Model(File&& file, const std::string& meshName) {
    file.load(*this, meshName);
  }

  uint64_t Model::getNumPoses() {
    return numPoses;
  }

  glm::mat4 Model::getJointTransform(size_t joint, uint64_t currentPose, float seconds) {

    float secondsUsed = seconds;

    if (seconds == 0.0f) {
      for (auto& anim : joints[joint].animations) {
        if (anim.times.size() > currentPose) {
          secondsUsed = anim.times[currentPose];
          break;
        }
      }
    }

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

    glm::mat4 parentTransform(1.0f);
    if (parentFound) {
      parentTransform = getJointTransform(idx, currentPose, secondsUsed);
    }

    glm::mat4 translation = glm::translate(glm::mat4(1.0f), joints[joint].translation);
    glm::mat4 rotation = glm::toMat4(joints[joint].rotation);
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), joints[joint].scale);

    bool firstT = true, firstR = true, firstS = true;
    for (auto& animation : joints[joint].animations) {
      auto poseUsed = currentPose;
      bool foundPose = false;

      uint32_t tidx = 0;
      for (auto& time : animation.times) {
        if (time >= secondsUsed) {
          poseUsed = tidx;
          foundPose = true;
          break;
        }
        ++tidx;
      }

      if (foundPose) {
        if (animation.translationAnimation.size() > poseUsed) {
          if (firstT) {
            translation = glm::translate(glm::mat4(1.0f), animation.translationAnimation[poseUsed]);
            firstT = false;
          }
          else {
            translation *= glm::translate(glm::mat4(1.0f), animation.translationAnimation[poseUsed]);
          }
        }
        if (animation.rotationAnimation.size() > poseUsed) {
          if (firstR) {
            rotation = glm::toMat4(animation.rotationAnimation[poseUsed]);
            firstR = false;
          }
          else {
            rotation *= glm::toMat4(animation.rotationAnimation[poseUsed]);
          }
        }
        if (animation.scaleAnimation.size() > poseUsed) {
          if (firstS) {
            scale = glm::scale(glm::mat4(1.0f), animation.scaleAnimation[poseUsed]);
            firstS = false;
          }
          else {
            scale *= glm::scale(glm::mat4(1.0f), animation.scaleAnimation[poseUsed]);
          }
        }
      }
    }

    return parentTransform * translation * rotation * scale;
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
