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
#include "WavefrontFile.hpp"

namespace small3d {

  Model::Model() {
  }

  Model::Model(const std::string& fileLocation) {
    WavefrontFile wf(fileLocation);
    wf.load(*this);
  }

  Model::Model(const std::string& fileLocation, const std::string& meshName) {
    GlbFile glb(fileLocation);
    bool loaded = false;
    for (auto& meshToken : glb.getChildTokens(glb.getToken("meshes"))) {

      if (glb.getChildToken(meshToken, "name")->value == meshName) {
        auto primitives = glb.getChildTokens(
          glb.getChildToken(meshToken, "primitives"));
        auto attributes = glb.getChildTokens(glb.getChildToken(primitives[0], "attributes"));

        for (auto attribute : attributes) {

          if (attribute->name == "POSITION") {

            auto data = glb.getBufferByAccessor(std::stoi(attribute->value));

            auto numFloatsInData = data.size() / 4;

            vertexData.resize(numFloatsInData + numFloatsInData / 3); // Add 1 float for each 3 for 
                                                                      // the w component of each vector

            vertexDataByteSize = static_cast<uint32_t>(vertexData.size() * 4); // Each vertex component is 4 bytes
            size_t vertexPos = 0;

            for (size_t idx = 0; idx < numFloatsInData; ++idx) {
              memcpy(&vertexData[vertexPos], &data[idx * 4], 4);
              ++vertexPos;
              if (idx > 0 && (idx + 1) % 3 == 0) {
                vertexData[vertexPos] = 1.0f;
                ++vertexPos;
              }
            }
          }

          if (attribute->name == "NORMAL") {
            auto data = glb.getBufferByAccessor(std::stoi(attribute->value));
            normalsData.resize(data.size() / 4);
            normalsDataByteSize = static_cast<uint32_t>(data.size());
            memcpy(&normalsData[0], &data[0], data.size());

          }

          if (attribute->name == "TEXCOORD_0") {
            auto data = glb.getBufferByAccessor(std::stoi(attribute->value));
            textureCoordsData.resize(data.size() / 4);
            textureCoordsDataByteSize = static_cast<uint32_t>(data.size());
            memcpy(&textureCoordsData[0], &data[0], data.size());
          }

          if (attribute->name == "JOINTS_0") {
            auto data = glb.getBufferByAccessor(std::stoi(attribute->value));
            jointData.resize(data.size());
            jointDataByteSize = static_cast<uint32_t>(data.size());
            memcpy(&jointData[0], &data[0], data.size());
          }

          if (attribute->name == "WEIGHTS_0") {
            auto data = glb.getBufferByAccessor(std::stoi(attribute->value));
            weightData.resize(data.size() / 4);
            weightDataByteSize = static_cast<uint32_t>(data.size());
            memcpy(&weightData[0], &data[0], data.size());
          }

        }
        auto data = glb.getBufferByAccessor(std::stoi(glb.getChildToken(primitives[0], "indices")->value));
        indexData.resize(data.size() / 2);
        indexDataByteSize = static_cast<uint32_t>(indexData.size() * 4); // 4 because, even though each index is read in 16 bits,
                                                                         // it is stored in 32 bits
        uint16_t indexBuf = 0;
        for (size_t idx = 0; idx < indexData.size(); ++idx) {
          memcpy(&indexBuf, &data[2 * idx], 2);
          indexData[idx] = static_cast<uint32_t>(indexBuf);
        }

        auto materialToken = glb.getChildToken(primitives[0], "material");
        if (materialToken != nullptr) {

          uint32_t materialIndex = std::stoi(materialToken->value);

          auto materialToken = glb.getChildTokens(glb.getToken("materials"))[materialIndex];

          auto metallicRoughnessToken = glb.getChildToken(materialToken, "pbrMetallicRoughness");
          if (metallicRoughnessToken != nullptr) {
            auto baseColorTextureToken = glb.getChildToken(metallicRoughnessToken, "baseColorTexture");
            if (baseColorTextureToken != nullptr) {
              uint32_t textureIndex = std::stoi(glb.getChildToken(baseColorTextureToken, "index")->value);
              uint32_t sourceIndex = std::stoi(glb.getChildToken(glb.getChildTokens(glb.getToken("textures"))[textureIndex], "source")->value);
              auto imageToken = glb.getChildTokens(glb.getToken("images"))[sourceIndex];
              if (glb.getChildToken(imageToken, "mimeType")->value == "image/png") {
                auto imageData = glb.getBufferByView(std::stoi(glb.getChildToken(imageToken, "bufferView")->value));

                defaultTextureImage = std::shared_ptr<Image>(new Image(imageData));
                imageData.clear();

              }
              else {
                LOGINFO("Warning! Only PNG images embedded in .glb files can be read. Texture ignored.");
              }
            }
          }
        }

        loaded = true;
        break;
      }
    }

    if (!loaded) throw std::runtime_error("Could not load mesh " + meshName + " from " + fileLocation);

    LOGDEBUG("Loaded mesh " + meshName + " from " + fileLocation);

    if (glb.existNode(meshName)) {
      auto meshNode = glb.getNode(meshName);
      if (!meshNode.noSkin && glb.existSkin(meshNode.skin)) {
        auto skin = glb.getSkin(meshNode.skin);

        if (skin.joints.size() > MAX_JOINTS_SUPPORTED) {
          throw std::runtime_error("Found more than the maximum of " +
            std::to_string(MAX_JOINTS_SUPPORTED) + " supported joints.");
        }

        auto inverseBindMatrices = glb.getBufferByAccessor(skin.inverseBindMatrices);
        uint64_t idx = 0;
        for (auto jointIdx : skin.joints) {
          Joint j;

          memcpy(&j.inverseBindMatrix, &inverseBindMatrices[idx * 16 * 4], 16 * 4);
          auto jointNode = glb.getNode(jointIdx);
          j.node = jointIdx;
          j.name = jointNode.name;
          j.rotation = jointNode.rotation;
          j.scale = jointNode.scale;
          j.translation = jointNode.translation;
          j.children = jointNode.children;
          joints.push_back(j);

          ++idx;
        }

        uint32_t animationIdx = 0;
        while (glb.existAnimation(animationIdx)) {
          auto animation = glb.getAnimation(animationIdx);
          ++animationIdx;

          for (auto& channel : animation.channels) {
            if (channel.target.path == "rotation") {
              auto sampler = animation.samplers[channel.sampler];

              auto output = glb.getBufferByAccessor(sampler.output);
              for (auto& joint : joints) {
                if (joint.node == channel.target.node) {
                  joint.rotationAnimation.resize(output.size() / sizeof(glm::quat));
                  memcpy(&joint.rotationAnimation[0], &output[0], output.size());
                  if (numPoses < joint.rotationAnimation.size())
                    numPoses = joint.rotationAnimation.size();

                  if (joint.animTime.size() == 0) {
                    auto input = glb.getBufferByAccessor(sampler.input);
                    joint.animTime.resize(input.size() / 4);
                    memcpy(&joint.animTime[0], &input[0], input.size());
                  }
                }
              }
            }

            if (channel.target.path == "translation") {
              auto sampler = animation.samplers[channel.sampler];
              auto input = glb.getBufferByAccessor(sampler.input);
              auto output = glb.getBufferByAccessor(sampler.output);
              for (auto& joint : joints) {
                if (joint.node == channel.target.node) {
                  joint.translationAnimation.resize(output.size() / sizeof(glm::vec3));
                  memcpy(&joint.translationAnimation[0], &output[0], output.size());
                  if (numPoses < joint.translationAnimation.size())
                    numPoses = joint.translationAnimation.size();
                }
              }
            }

          }
        }
      }
    }

  }

  uint32_t Model::getCurrentPoseIdx() {
    return currentPose;
  }

  void Model::animate() {
    if (numPoses != 0) {
      ++currentPose;
      if (currentPose == numPoses) {
        currentPose = 0;
      }
      for (auto& joint : joints) {
        if (joint.rotationAnimation.size() > currentPose) {
          joint.currRotation = joint.rotationAnimation[currentPose];
        }
        if (joint.translationAnimation.size() > currentPose) {
          joint.currTranslation = joint.translationAnimation[currentPose];
        }
      }
    }
  }

  glm::mat4 Model::getJointTransform(uint64_t joint) {

    glm::mat4 parentTransform(1.0f);
    glm::mat4 transform(1.0f);

    uint64_t idx = 0;
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
      parentTransform = getJointTransform(idx);
    }

    if (currentPose < joints[joint].rotationAnimation.size() && currentPose < joints[joint].translationAnimation.size()) {
      transform = glm::translate(glm::mat4(1.0f), joints[joint].translationAnimation[currentPose]) *
        glm::toMat4(joints[joint].rotationAnimation[currentPose]);
    }
    else {
      transform = glm::translate(glm::mat4(1.0f), joints[joint].translation) *
        glm::toMat4(joints[joint].rotation) *
        glm::scale(glm::mat4(1.0f), joints[joint].scale);
    }

    return parentTransform * transform;
  }
}
