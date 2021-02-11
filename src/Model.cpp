/*
 *  Model.cpp
 *
 *  Created on: 2017/07/13
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#include <stdexcept>
#include <fstream>
#include <unordered_map>
#include <memory>
#include "GetTokens.hpp"
#include "Model.hpp"
#include "BasePath.hpp"
#include "Logger.hpp"

#ifdef __ANDROID__
#include "vkzos.h"
#include <streambuf>
#include <istream>

struct membuf : std::streambuf
{
  membuf(char* begin, char* end) {
    this->setg(begin, begin, end);
  }
};
#endif

namespace small3d {

  void Model::loadVertexData() {
    // 4 components per vertex
    this->vertexDataByteSize = static_cast<int>(4 * vertices.size() *
      sizeof(float));

    this->vertexData.clear();

    int idx = 0;
    for (auto vertex = vertices.begin(); vertex != vertices.end(); ++vertex) {
      for (unsigned long coordIdx = 0; coordIdx != 3; ++coordIdx) {
        this->vertexData.push_back(vertex->at(coordIdx));
        ++idx;
      }
      this->vertexData.push_back(1.0f);
      ++idx;
    }
  }

  void Model::loadIndexData() {
    // 3 indices per face
    int numIndexes = (int)(facesVertexIndices.size() * 3);
    this->indexDataByteSize = numIndexes * sizeof(uint32_t);

    this->indexData.clear();

    for (auto face = facesVertexIndices.begin();
      face != facesVertexIndices.end(); ++face) {

      for (int indexIdx = 0; indexIdx != 3; ++indexIdx) {
        this->indexData.push_back(face->at((unsigned long)indexIdx)
          - 1); // -1 because Wavefront indexes
                // are not 0 based

      }
    }
  }

  void Model::loadNormalsData() {

    // Create an array of normal components which corresponds
    // by index to the array of vertex components

    if (this->vertexData.size() == 0) {
      throw std::runtime_error("There are no vertices or vertex data has not "
        "yet been created.");
    }

    // 3 components per vertex (a single index for vertices, normals and texture
    // coordinates is passed to OpenGL, so normals data will be aligned to
    // vertex data according to the vertex index)

    this->normalsDataByteSize = static_cast<int>(3 * vertices.size() *
      sizeof(float));

    this->normalsData = std::vector<float>(3 * vertices.size(), 0.0f);

    int faceVertexArrayIndex = 0;
    for (auto faceVertexIndex = facesVertexIndices.begin();
      faceVertexIndex != facesVertexIndices.end(); ++faceVertexIndex) {
      for (int vertexIndex = 0; vertexIndex != 3; ++vertexIndex) {

        for (int normalsDataComponent = 0; normalsDataComponent != 3;
          ++normalsDataComponent) {
          this->normalsData[3 * (faceVertexIndex->at(vertexIndex) - 1)
            + normalsDataComponent] =
            normals.at(
              (unsigned long)
              (facesNormalIndices.at((unsigned long)
                faceVertexArrayIndex)[vertexIndex]
                - 1))[normalsDataComponent];
        }
      }
      ++faceVertexArrayIndex;
    }
  }

  void Model::loadTextureCoordsData() {

    // Create an array of texture coordinates components which corresponds
    // by index to the array of vertex components
    // Doing this even if there are no texture coordinates in the file because
    // Vulkan is a bit rigid about missing data in the shaders and it crashes if 
    // a bound buffer is empty.

    if (this->vertexData.size() == 0) {
      throw std::runtime_error("There are no vertices or vertex data has not "
        "yet been created.");
    }

    // 2 components per vertex (a single index for vertices, normals and
      // texture coordinates is passed to OpenGL, so texture coordinates data
      // will be aligned to vertex data according to the vertex index)
    this->textureCoordsDataByteSize = (int)(2 * vertices.size() *
      sizeof(float));

    this->textureCoordsData = std::vector<float>(2 * vertices.size());

    if (!textureCoords.empty()) {


      int faceVertexArrayIndex = 0;

      for (auto faceVertexIndex = facesVertexIndices.begin();
        faceVertexIndex != facesVertexIndices.end();
        ++faceVertexIndex) {
        for (int vertexIndex = 0; vertexIndex != 3; ++vertexIndex) {

          for (int textureCoordsComponent = 0;
            textureCoordsComponent != 2; ++textureCoordsComponent) {
            this->textureCoordsData[2 * (faceVertexIndex->at(vertexIndex) - 1)
              + textureCoordsComponent] =
              textureCoords.at(
                static_cast<unsigned long>
                (textureCoordsIndices.at(faceVertexArrayIndex)
                  [vertexIndex]
            - 1))[textureCoordsComponent];
          }
        }
        ++faceVertexArrayIndex;
      }
    }
  }

  void Model::correctDataVectors() {

    std::unique_ptr<std::unordered_map<int, int> >
      vertexUVPairs(new std::unordered_map<int, int>());

    int numIndexes = static_cast<int>(facesVertexIndices.size());

    for (int idx = 0; idx < numIndexes; ++idx) {

      for (int vertexIndex = 0; vertexIndex != 3; ++vertexIndex) {

        auto vertexUVPair = vertexUVPairs->
          find(facesVertexIndices[idx][vertexIndex]);
        if (vertexUVPair != vertexUVPairs->end()) {
          if (vertexUVPair->second !=
            textureCoordsIndices.at(idx)[vertexIndex]) {
            // duplicate corresponding vertex data entry and point the vertex
            // index to the new tuple
            std::vector<float> v;
            // -1 because at this stage the indexes are still as exported from
            // Blender, meaning 1-based and not 0-based
            v.push_back(vertices[facesVertexIndices[idx][vertexIndex] - 1][0]);
            v.push_back(vertices[facesVertexIndices[idx][vertexIndex] - 1][1]);
            v.push_back(vertices[facesVertexIndices[idx][vertexIndex] - 1][2]);
            vertices.push_back(v);

            facesVertexIndices[idx][vertexIndex] = static_cast<int>
              (vertices.size());

            vertexUVPairs->
              insert(std::make_pair(facesVertexIndices[idx][vertexIndex],
                textureCoordsIndices[idx][vertexIndex]));
          }
          // So we don't add a pair if the exact same pair already exists. We
          // do if it does not (see below) or if the vertex index number exists
          // in a pair with a different texture coordinates index number (see
          // above)
        }
        else {
          vertexUVPairs->
            insert(std::make_pair(facesVertexIndices[idx][vertexIndex],
              textureCoordsIndices[idx][vertexIndex]));
        }
      }
    }
  }


  void Model::clear() {
    vertices.clear();
    facesVertexIndices.clear();
    normals.clear();
    facesNormalIndices.clear();
    textureCoords.clear();
    textureCoordsIndices.clear();
  }

  Model::Model() {
  }

  Model::Model(const std::string fileLocation) {
    std::string line;

#ifdef __ANDROID__
    AAsset* asset = AAssetManager_open(vkz_android_app->activity->assetManager,
      fileLocation.c_str(),
      AASSET_MODE_STREAMING);
    if (!asset) throw std::runtime_error("Opening asset " + fileLocation +
      " has failed!");
    off_t length;
    length = AAsset_getLength(asset);
    const void* buffer = AAsset_getBuffer(asset);
    membuf sbuf((char*)buffer, (char*)buffer + sizeof(char) * length);
    std::istream in(&sbuf);
    if (in) {
      clear();
      while (std::getline(in, line)) {
#else
    std::string fullPath = getBasePath() + fileLocation;

    std::ifstream file(fullPath.c_str());

    if (file.is_open()) {
      clear();
      while (std::getline(file, line)) {
#endif
        if (line[0] == 'v' || line[0] == 'f') {
          std::vector<std::string> tokens;

          int numTokens = getTokens(line, ' ', tokens);

          int idx = 0;

          if (line[0] == 'v' && line[1] == 'n') {
            // get vertex normal
            std::vector<float> vn;

            for (int tokenIdx = 0; tokenIdx < numTokens; ++tokenIdx) {
              std::string t = tokens[tokenIdx];
              if (idx > 0)   // The first token is the vertex normal indicator
              {
                vn.push_back(static_cast<float>(atof(t.c_str())));
              }
              ++idx;
            }
            normals.push_back(vn);
          }
          else if (line[0] == 'v' && line[1] == 't') {
            std::vector<float> vt;

            for (int tokenIdx = 0; tokenIdx < numTokens; ++tokenIdx) {
              std::string t = tokens[tokenIdx];
              if (idx > 0)   // The first token is the vertex texture
                 // coordinate indicator.
              {
                vt.push_back(static_cast<float>(atof(t.c_str())));
              }
              ++idx;
            }

            vt[1] = 1.0f - vt[1]; // OpenGL's y direction for textures is the
                            // opposite of that of Blender's, so an
                            // inversion is needed
            textureCoords.push_back(vt);
          }
          else if (line[0] == 'v') {
            // get vertex
            std::vector<float> v;

            for (int tokenIdx = 0; tokenIdx < numTokens; ++tokenIdx) {
              std::string t = tokens[tokenIdx];
              if (idx > 0)   // The first token is the vertex indicator
              {
                v.push_back(static_cast<float>(atof(t.c_str())));
              }
              ++idx;
            }
            vertices.push_back(v);
          }
          else {
            // get vertex index
            std::vector<int> v = std::vector<int>(3, 0);
            std::vector<int> n;
            std::vector<int> textC;

            for (int tokenIdx = 0; tokenIdx < numTokens; ++tokenIdx) {
              std::string t = tokens[tokenIdx];

              if (idx > 0)   // The first token is face indicator
              {
                if (t.find("//") != std::string::npos)   // normal index
                  // contained in the
                  // string
                {
                  v[idx - 1] = atoi(
                    t.substr(0, t.find("//")).c_str());
                  n.push_back(atoi(
                    t.substr(t.find("//") + 2).c_str()));
                }
                else if (t.find("/") != std::string::npos
                  && t.find("//") ==
                  std::string::npos) // normal and texture coordinate
                 // index are contained in the string
                {
                  std::vector<std::string> components;
                  int numComponents = getTokens(t, '/', components);

                  int componentIdx = 0;

                  for (int compIdx = 0; compIdx < numComponents; ++compIdx) {
                    std::string component = components[compIdx];
                    switch (componentIdx) {
                    case 0:
                      v[idx - 1] = atoi(component.c_str());
                      break;
                    case 1:
                      textC.push_back(atoi(
                        component.c_str()));
                      break;
                    case 2:
                      n.push_back(atoi(component.c_str()));
                      break;
                    default:
                      throw std::runtime_error("Unexpected component index "
                        "number while parsing "
                        "Wavefront file.");
                      break;
                    }
                    ++componentIdx;
                  }

                }

                else   // just the vertex index is contained in the string
                {
                  v[idx - 1] = atoi(t.c_str());
                }
              }
              ++idx;
            }
            facesVertexIndices.push_back(v);
            if (!n.empty())
              facesNormalIndices.push_back(n);
            if (!textC.empty())
              textureCoordsIndices.push_back(textC);
          }

        }
      }
#ifdef __ANDROID__
      AAsset_close(asset);
#else
      file.close();
#endif
      if (textureCoords.size() > 0) {
        this->correctDataVectors();
      }
      // Generate the data and delete the initial buffers
      this->loadVertexData();
      this->loadIndexData();
      this->loadNormalsData();
      this->loadTextureCoordsData();
      this->clear();
    }
    else
      throw std::runtime_error("Could not open file " + fileLocation);
  }

  Model::Model(const std::string & fileLocation, const std::string & meshName) {
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

        loaded = true;
        break;
      }
    }

    if (!loaded) throw std::runtime_error("Could not load mesh " + meshName + " from " + fileLocation);

    LOGDEBUG("Loaded mesh " + meshName + " from " + fileLocation);

    if (glb.existNode(meshName)) {
      auto meshNode = glb.getNode(meshName);
      if (glb.existSkin(meshNode.skin)) {
        auto skin = glb.getSkin(meshNode.skin);

        if (skin.joints.size() > MAX_JOINTS_SUPPORTED) {
          throw std::runtime_error("Found more than the maximum of " + 
            std::to_string(MAX_JOINTS_SUPPORTED) + " supported joints.");
        }

        auto inverseBindMatrices = glb.getBufferByAccessor(skin.inverseBindMatrices);

        for (auto jointIdx : skin.joints) {
          Joint j;

          memcpy(&j.inverseBindMatrix, &inverseBindMatrices[static_cast<uint32_t>(jointIdx) * 16 * 4], 16 * 4);
          auto jointNode = glb.getNode(jointIdx);
          j.id = jointIdx;
          j.name = jointNode.name;
          j.rotation = jointNode.rotation;
          j.scale = jointNode.scale;
          j.translation = jointNode.translation;
          j.children = jointNode.children;
          joints.push_back(j);
        }
      }
    }

  }


}
