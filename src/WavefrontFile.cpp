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
#include "BasePath.hpp"
#include <fstream>
#include "GetTokens.hpp"

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
  
  void WavefrontFile::loadVertexData(std::vector<float>& vertexData) {

    vertexData.clear();

    int idx = 0;
    for (auto vertex = vertices.begin(); vertex != vertices.end(); ++vertex) {
      for (unsigned long coordIdx = 0; coordIdx != 3; ++coordIdx) {
        vertexData.push_back(vertex->at(coordIdx));
        ++idx;
      }
      vertexData.push_back(1.0f);
      ++idx;
    }
  }

  void WavefrontFile::loadIndexData(std::vector<uint32_t>& indexData) {

    if (!onlyTriangles) {
      throw std::runtime_error("Cannot load indices from " + fileLocation + " because"
      " it does not only contain triangles.");
    }

    indexData.clear();

    for (auto face = facesVertexIndices.begin();
      face != facesVertexIndices.end(); ++face) {

      for (int indexIdx = 0; indexIdx != 3; ++indexIdx) {
        indexData.push_back(face->at((unsigned long)indexIdx)
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

  void WavefrontFile::correctDataVectors() {

    std::unordered_map<int, int> vertexUVPairs;

    uint64_t numIndexes = facesVertexIndices.size();

    for (uint64_t idx = 0; idx < numIndexes; ++idx) {

      for (uint64_t vertexIndex = 0; vertexIndex != 3; ++vertexIndex) {

        auto vertexUVPair = vertexUVPairs.
          find(facesVertexIndices[idx][vertexIndex]);
        if (vertexUVPair != vertexUVPairs.end()) {
          if (vertexUVPair->second !=
            textureCoordsIndices.at(idx)[vertexIndex]) {
            // duplicate corresponding vertex data entry and point the vertex
            // index to the new tuple
            std::vector<float> v;
            // -1 because at this stage the indexes are still as exported from
            // Blender, meaning 1-based and not 0-based
            v.push_back(vertices[static_cast<uint64_t>(facesVertexIndices[idx][vertexIndex] - 1)][0]);
            v.push_back(vertices[static_cast<uint64_t>(facesVertexIndices[idx][vertexIndex] - 1)][1]);
            v.push_back(vertices[static_cast<uint64_t>(facesVertexIndices[idx][vertexIndex] - 1)][2]);
            vertices.push_back(v);

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

  WavefrontFile::WavefrontFile(const std::string& fileLocation) {
    
    this->fileLocation = fileLocation;

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
      while (std::getline(in, line)) {
#else
    std::string fullPath = getBasePath() + fileLocation;

    std::ifstream file(fullPath.c_str());

    if (file.is_open()) {
      while (std::getline(file, line)) {
#endif
        if (line[0] == 'v' || line[0] == 'f') {
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
                vn.push_back(static_cast<float>(atof(t.c_str())));
              }
              ++idx;
            }
            normals.push_back(vn);
          }
          else if (line[0] == 'v' && line[1] == 't') {
            std::vector<float> vt;

            for (uint32_t tokenIdx = 0; tokenIdx < numTokens; ++tokenIdx) {
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

            for (uint32_t tokenIdx = 0; tokenIdx < numTokens; ++tokenIdx) {
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
                  v.push_back(atoi(
                    t.substr(0, t.find("//")).c_str()));
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
                      v.push_back(atoi(component.c_str()));
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
                  v.push_back(atoi(t.c_str()));
                }
              }
              ++idx;
            }
            facesVertexIndices.push_back(v);
            if (v.size() != 3) onlyTriangles = false;

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
      
    }
    else
      throw std::runtime_error("Could not open file " + fileLocation);
  }

  void WavefrontFile::load(Model& model) {
    
    loadVertexData(model.vertexData);
    loadIndexData(model.indexData);
    loadNormalsData(model.normalsData, model.vertexData);
    loadTextureCoordsData(model.textureCoordsData, model.vertexData);

    model.vertexDataByteSize = static_cast<uint32_t>(model.vertexData.size() * sizeof(float));
    model.indexDataByteSize = static_cast<uint32_t>(model.indexData.size() * sizeof(uint32_t));
    model.normalsDataByteSize = static_cast<uint32_t>(model.normalsData.size() * sizeof(float));
    model.textureCoordsDataByteSize = static_cast<uint32_t>(model.textureCoordsData.size() * sizeof(float));
    
  }

  void WavefrontFile::load(BoundingBoxSet& boundingBoxSet) {
    boundingBoxSet.vertices = this->vertices;
    boundingBoxSet.facesVertexIndexes = this->facesVertexIndices;
  }

}
