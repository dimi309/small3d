/**
 *  WavefrontFile.cpp
 *
 *  Created on: 2021/02/22
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#include "WavefrontFile.hpp"
#include <stdexcept>
#include <unordered_map>
#include <fstream>
#include <regex>

#ifdef __ANDROID__
#include "small3d_android.h"
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

  int WavefrontFile::getTokens(const std::string& input, const char sep,
    std::vector<std::string>& tokens) {
    size_t curPos = 0;
    int count = 0;

    size_t length = input.length();

    for (size_t idx = 0; idx < length; ++idx) {
      if (input[idx] == sep) {
        ++count;
      }
    }
    ++count;

    for (int idx = 0; idx < count; ++idx) {
      // last one
      if (idx == count - 1) {
        tokens.emplace_back(input.substr(curPos));
      }
      else {

        size_t foundPos = input.find(sep, curPos);
        tokens.emplace_back(input.substr(curPos, foundPos - curPos));
        curPos = foundPos + 1;
      }
    }
    return count;
  }

  void WavefrontFile::loadVertexData(std::vector<float>& vertexData) {

    vertexData.clear();

    int idx = 0;
    for (auto vertex = vertices.begin(); vertex != vertices.end(); ++vertex) {
      for (unsigned long coordIdx = 0; coordIdx != 3; ++coordIdx) {
        vertexData.emplace_back(vertex->at(coordIdx));
        ++idx;
      }
      vertexData.emplace_back(1.0f);
      ++idx;
    }
  }

  void WavefrontFile::loadIndexData(std::vector<uint16_t>& indexData) {

    if (!onlyTriangles) {
      throw std::runtime_error("Cannot load indices from " + fullPath + " because"
        " it does not only contain triangles.");
    }

    indexData.clear();

    for (auto face = facesVertexIndices.begin();
      face != facesVertexIndices.end(); ++face) {

      for (int indexIdx = 0; indexIdx != 3; ++indexIdx) {
        indexData.emplace_back(face->at((unsigned long)indexIdx)
          - 1); // -1 because Wavefront indices
        // are not 0 based
      }
    }

  }

  void WavefrontFile::loadNormalsData(std::vector<float>& normalsData, const std::vector<float>& vertexData) {

    // Create an array of normals, corresponding by index to the 
    // array of vertices. 

    if (vertexData.size() == 0) {
      throw std::runtime_error("Cannot create normals data when there is no vertex data.");
    }

    // 3 components per vertex 

    normalsData = std::vector<float>(3 * vertices.size(), 0.0f);

    int faceVertexArrayIndex = 0;
    for (auto faceVertexIndex = facesVertexIndices.begin();
      faceVertexIndex != facesVertexIndices.end(); ++faceVertexIndex) {
      for (uint64_t vertexIndex = 0; vertexIndex != 3; ++vertexIndex) {

        for (uint64_t normalsDataComponent = 0; normalsDataComponent != 3;
          ++normalsDataComponent) {
          normalsData[3 * static_cast<uint64_t>((faceVertexIndex->at(vertexIndex) - 1))
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

  void WavefrontFile::loadTextureCoordsData(std::vector<float>& textureCoordsData, const std::vector<float>& vertexData) {

    // Create an array of texture coordinates components which corresponds
    // by index to the array of vertex components

    if (vertexData.size() == 0) {
      throw std::runtime_error("Cannot create texture coords data when there is no vertex data.");
    }

    textureCoordsData = std::vector<float>(2 * vertices.size(), 0.0f);

    if (!textureCoords.empty()) {

      int faceVertexArrayIndex = 0;

      for (auto faceVertexIndex = facesVertexIndices.begin();
        faceVertexIndex != facesVertexIndices.end();
        ++faceVertexIndex) {
        for (uint64_t vertexIndex = 0; vertexIndex != 3; ++vertexIndex) {

          for (uint64_t textureCoordsComponent = 0;
            textureCoordsComponent != 2; ++textureCoordsComponent) {
            textureCoordsData[2 * static_cast<uint64_t>((faceVertexIndex->at(vertexIndex) - 1))
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

  void WavefrontFile::loadMaterial(const std::string& filePath, const std::string& name) {

    std::string line;
    bool inMaterial = false;

#ifdef __ANDROID__
    AAsset* asset = AAssetManager_open(small3d_android_app->activity->assetManager,
      filePath.c_str(),
      AASSET_MODE_STREAMING);
    if (!asset) {
      LOGDEBUG("Could not open file " + filePath);
      return;
    }

    off_t length;
    length = AAsset_getLength(asset);
    const void* buffer = AAsset_getBuffer(asset);
    membuf sbuf((char*)buffer, (char*)buffer + sizeof(char) * length);
    std::istream in(&sbuf);
    if (in) {
      while (std::getline(in, line)) {
#else

    std::ifstream file(filePath.c_str());

    if (file.is_open()) {
      while (std::getline(file, line)) {

#endif
        if (inMaterial) {
          if (line.substr(0, 6) == "newmtl") {
            inMaterial = false;
          }
          else {
            std::vector<std::string> tokens;
            getTokens(line, ' ', tokens);
            if (tokens.size() > 0) {

              if (tokens[0] == "Ns") {
                material.specularExponent = atof(tokens[1].c_str());
              }
              else if (tokens[0] == "Ka") {
                material.ambientColour = glm::vec3(atof(tokens[1].c_str()), atof(tokens[2].c_str()), atof(tokens[3].c_str()));
              }
              else if (tokens[0] == "Kd") {
                material.diffuseColour = glm::vec3(atof(tokens[1].c_str()), atof(tokens[2].c_str()), atof(tokens[3].c_str()));
              }
              else if (tokens[0] == "Ks") {
                material.specularColour = glm::vec3(atof(tokens[1].c_str()), atof(tokens[2].c_str()), atof(tokens[3].c_str()));
              }
              else if (tokens[0] == "Ke") {
                material.emissiveCoefficient = glm::vec3(atof(tokens[1].c_str()), atof(tokens[2].c_str()), atof(tokens[3].c_str()));
              }
              else if (tokens[0] == "Ni") {
                material.optDensIndexRef = atof(tokens[1].c_str());
              }
              else if (tokens[0] == "d" || tokens[0] == "Tr") {
                material.alpha = atof(tokens[1].c_str());
              }
            }
          }
        }
        else if (line.substr(0, 6) == "newmtl") {
          inMaterial = true;
        }

      }
#ifdef __ANDROID__
      AAsset_close(asset);
#else
      file.close();
#endif
     
    }
    else {
      LOGDEBUG("Could not open file " + filePath);
    }
  }

  void WavefrontFile::correctDataVectors() {

    std::unordered_map<int, int> vertexUVPairs;

    uint64_t numIndices = facesVertexIndices.size();

    for (uint64_t idx = 0; idx < numIndices; ++idx) {

      for (uint64_t vertexIndex = 0; vertexIndex != 3; ++vertexIndex) {

        auto vertexUVPair = vertexUVPairs.
          find(facesVertexIndices[idx][vertexIndex]);
        if (vertexUVPair != vertexUVPairs.end()) {
          if (vertexUVPair->second !=
            textureCoordsIndices.at(idx)[vertexIndex]) {
            // duplicate corresponding vertex data entry and point the vertex
            // index to the new tuple
            std::vector<float> v;
            // -1 because at this stage the indices are still as exported from
            // Blender, meaning 1-based and not 0-based
            v.emplace_back(vertices[static_cast<uint64_t>(facesVertexIndices[idx][vertexIndex] - 1)][0]);
            v.emplace_back(vertices[static_cast<uint64_t>(facesVertexIndices[idx][vertexIndex] - 1)][1]);
            v.emplace_back(vertices[static_cast<uint64_t>(facesVertexIndices[idx][vertexIndex] - 1)][2]);
            vertices.emplace_back(v);

            facesVertexIndices[idx][vertexIndex] = static_cast<int>
              (vertices.size());

            vertexUVPairs.
              insert(std::make_pair(facesVertexIndices[idx][vertexIndex],
                textureCoordsIndices[idx][vertexIndex]));
          }
          // So we don't add a pair if the exact same pair already exists. We
          // do if it does not (see below) or if the vertex index number exists
          // in a pair with a different texture coordinates index number (see
          // above)
        }
        else {
          vertexUVPairs.
            insert(std::make_pair(facesVertexIndices[idx][vertexIndex],
              textureCoordsIndices[idx][vertexIndex]));
        }
      }
    }
  }

  WavefrontFile::WavefrontFile(const std::string & filePath) : File(filePath) {

    std::string line;

#ifdef __ANDROID__
    AAsset* asset = AAssetManager_open(small3d_android_app->activity->assetManager,
      fullPath.c_str(),
      AASSET_MODE_STREAMING);
    if (!asset) throw std::runtime_error("Opening asset " + fullPath +
      " has failed!");
    off_t length;
    length = AAsset_getLength(asset);
    const void* buffer = AAsset_getBuffer(asset);
    membuf sbuf((char*)buffer, (char*)buffer + sizeof(char) * length);
    std::istream in(&sbuf);
    if (in) {
      while (std::getline(in, line)) {
#else

    std::ifstream file(fullPath.c_str());

    if (file.is_open()) {
      while (std::getline(file, line)) {
#endif

        if (line[0] == 'o') {
          std::vector<std::string> tokens;
          uint32_t numTokens = getTokens(line, ' ', tokens);
          if (numTokens > 1) {
            std::string objName = std::regex_replace(tokens[1], std::regex("\\r"), "");
            objectNames.emplace_back(objName);
            objectStartFaceIdx.insert(std::pair<std::string, size_t>(objName, facesVertexIndices.size()));
          }
        }
        else if (line[0] == 'v' || line[0] == 'f') {
          std::vector<std::string> tokens;

          uint32_t numTokens = getTokens(line, ' ', tokens);

          uint32_t idx = 0;

          if (line[0] == 'v' && line[1] == 'n') {
            // get vertex normal
            std::vector<float> vn;

            for (uint32_t tokenIdx = 0; tokenIdx < numTokens; ++tokenIdx) {
              std::string t = tokens[tokenIdx];
              if (idx > 0)   // The first token is the vertex normal indicator
              {
                vn.emplace_back(static_cast<float>(atof(t.c_str())));
              }
              ++idx;
            }
            normals.emplace_back(vn);
          }
          else if (line[0] == 'v' && line[1] == 't') {
            std::vector<float> vt;

            for (uint32_t tokenIdx = 0; tokenIdx < numTokens; ++tokenIdx) {
              std::string t = tokens[tokenIdx];
              if (idx > 0)   // The first token is the vertex texture
                // coordinate indicator.
              {
                vt.emplace_back(static_cast<float>(atof(t.c_str())));
              }
              ++idx;
            }

            vt[1] = 1.0f - vt[1]; // OpenGL's y direction for textures is the
            // opposite of that of Blender's, so an
            // inversion is needed
            textureCoords.emplace_back(vt);
          }
          else if (line[0] == 'v') {
            // get vertex
            std::vector<float> v;

            for (uint32_t tokenIdx = 0; tokenIdx < numTokens; ++tokenIdx) {
              std::string t = tokens[tokenIdx];
              if (idx > 0)   // The first token is the vertex indicator
              {
                v.emplace_back(static_cast<float>(atof(t.c_str())));
              }
              ++idx;
            }
            vertices.emplace_back(v);
          }
          else {
            // get vertex index
            std::vector<uint32_t> v;
            std::vector<uint32_t> n;
            std::vector<uint32_t> textC;

            for (uint32_t tokenIdx = 0; tokenIdx < numTokens; ++tokenIdx) {
              std::string t = tokens[tokenIdx];

              if (idx > 0)   // The first token is face indicator
              {
                if (t.find("//") != std::string::npos)   // normal index
                  // contained in the
                  // string
                {
                  v.emplace_back(atoi(
                    t.substr(0, t.find("//")).c_str()));
                  n.emplace_back(atoi(
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
                      v.emplace_back(atoi(component.c_str()));
                      break;
                    case 1:
                      textC.emplace_back(atoi(
                        component.c_str()));
                      break;
                    case 2:
                      n.emplace_back(atoi(component.c_str()));
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
                  v.emplace_back(atoi(t.c_str()));
                }
              }
              ++idx;
            }
            facesVertexIndices.emplace_back(v);
            if (v.size() != 3) onlyTriangles = false;

            if (!n.empty())
              facesNormalIndices.emplace_back(n);
            if (!textC.empty())
              textureCoordsIndices.emplace_back(textC);
          }
        }
        else if (line.compare(0, 6, "mtllib") == 0) {
          if (materialFile != "") {
            LOGDEBUG("More than one material files detected. Ignoring.");
          }
          else {
            materialFile = line.substr(7);
          }
        }
        else if (line.compare(0, 6, "usemtl") == 0) {
#ifdef _WIN32
          auto dirLength = fullPath.find_last_of("\\");
#else
          auto dirLength = fullPath.find_last_of("/");
#endif
          if (dirLength > 0) {
            loadMaterial(fullPath.substr(0, dirLength + 1) + materialFile, line.substr(7));
          } else {
            loadMaterial(materialFile, line.substr(7));
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

    }
    else {
      throw std::runtime_error("Could not open file " + fullPath);
    }

  }

  void WavefrontFile::load(Model & model, const std::string & meshName) {

    loadVertexData(model.vertexData);
    loadIndexData(model.indexData);
    loadNormalsData(model.normalsData, model.vertexData);
    loadTextureCoordsData(model.textureCoordsData, model.vertexData);

    if (objectNames.size() != 0 || meshName != "") {
      size_t startFaceIdx = 0;
      size_t numFaces = facesVertexIndices.size();
      if (meshName != "") {
        bool found = false;
        for (size_t idx = 0; idx < objectNames.size(); ++idx) {
          if (objectNames[idx] == meshName) {
            found = true;
            startFaceIdx = objectStartFaceIdx.find(meshName)->second;
            if (idx != objectNames.size() - 1) {
              numFaces = objectStartFaceIdx.find(objectNames[idx + 1])->second;
            }
            break;
          }
        }
        if (!found) throw std::runtime_error("Mesh " + meshName + " not found in " + fullPath + ".");
      }
      else if (objectNames.size() > 1) {
        // Will just load the first mesh (object in Wavefront)
        numFaces = objectStartFaceIdx.find(objectNames[1])->second;
      }

      model.indexData = std::vector<uint16_t>(model.indexData.begin() + startFaceIdx * 3,
        model.indexData.begin() + numFaces * 3);

      size_t minIndex = model.indexData[0];
      for (auto i : model.indexData) if (i < minIndex) minIndex = i;

      size_t maxIndex = model.indexData[0];
      for (auto i : model.indexData) if (i > maxIndex) maxIndex = i;

      model.vertexData = std::vector<float>(model.vertexData.begin() + minIndex * 4,
        model.vertexData.begin() + (maxIndex + 1) * 4);

      model.normalsData = std::vector<float>(model.normalsData.begin() + minIndex * 3,
        model.normalsData.begin() + (maxIndex + 1) * 3);

      model.textureCoordsData = std::vector<float>(model.textureCoordsData.begin() + minIndex * 2,
        model.textureCoordsData.begin() + (maxIndex + 1) * 2);

      for (size_t idx = 0; idx < model.indexData.size(); ++idx) {
        model.indexData[idx] -= static_cast<uint32_t>(minIndex);
      }
    }

    model.vertexDataByteSize = static_cast<uint32_t>(model.vertexData.size() * sizeof(float));
    model.indexDataByteSize = static_cast<uint32_t>(model.indexData.size() * sizeof(uint16_t));
    model.normalsDataByteSize = static_cast<uint32_t>(model.normalsData.size() * sizeof(float));
    model.textureCoordsDataByteSize = static_cast<uint32_t>(model.textureCoordsData.size() * sizeof(float));
    model.material = material;
    LOGDEBUG("Loaded mesh " + meshName + " from " + fullPath);

  }

  std::vector<std::string> WavefrontFile::getMeshNames() {
    return objectNames;
  }

}
