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

    glm::mat4 parentTransform(1.0f);
    glm::mat4 transform(1.0f);

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

    float secondsUsed = seconds;

    if (seconds == 0.0f) {
      for (auto& anim : joints[joint].animations) {
        if (anim.times.size() > currentPose) {
          secondsUsed = anim.times[currentPose];
          break;
        }
      }
    }

    if (parentFound) {
      parentTransform = getJointTransform(idx, currentPose, secondsUsed);
    }

    auto pTranslation = joints[joint].translation;
    auto pRotation = joints[joint].rotation;
    auto pScale = joints[joint].scale;

    for (auto& animation : joints[joint].animations) {
      auto poseUsed = currentPose;
      if (secondsUsed != 0) {
        uint32_t tidx = 0;
        for (auto& time : animation.times) {
          if (time >= secondsUsed) {
            poseUsed = tidx;
            break;
          }
          ++tidx;
        }
      }
      if (poseUsed < animation.translationAnimation.size()) {
        pTranslation = animation.translationAnimation[poseUsed];
      }
      if (poseUsed < animation.rotationAnimation.size()) {
        pRotation = animation.rotationAnimation[poseUsed];
      }
      if (poseUsed < animation.scaleAnimation.size()) {
        pScale = animation.scaleAnimation[poseUsed];
      }
    }

    transform = glm::translate(glm::mat4(1.0f), pTranslation) *
      glm::toMat4(pRotation) *
      glm::scale(glm::mat4(1.0f), pScale);

    return parentTransform * transform;
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
