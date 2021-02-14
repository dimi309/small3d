/*
 *  BoundingBoxSet.cpp
 *
 *  Created on: 2014/10/19
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#include "BoundingBoxSet.hpp"
#include <fstream>
#include <stdexcept>
#include "GetTokens.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "BasePath.hpp"

#if defined(__ANDROID__)
#include "vkzos.h"
#include <streambuf>
#include <istream>
#endif

namespace small3d {

  /**
   * Constructor
   */

  BoundingBoxSet::BoundingBoxSet(const std::string fileLocation) {
    vertices.clear();
    facesVertexIndexes.clear();
    numBoxes = 0;

    if (fileLocation != "") {

      this->loadFromFile(getBasePath() + fileLocation);

    }

  }

#ifdef __ANDROID__
  struct membuf : std::streambuf
  {
    membuf(char* begin, char* end) {
      this->setg(begin, begin, end);
    }
  };

#endif


  void BoundingBoxSet::loadFromFile(std::string fileLocation) {
    if (vertices.size() != 0) {
      throw std::runtime_error("Illegal attempt to reload bounding boxes. "
        "Please use another object.");
    }
    std::string line;
#ifdef __ANDROID__
    AAsset* asset = AAssetManager_open(vkz_android_app->activity->assetManager,
      fileLocation.c_str(),
      AASSET_MODE_STREAMING);
    if (!asset) {
      throw std::runtime_error(
        "Opening asset " + fileLocation + " has failed!");
    }
    off_t length;
    length = AAsset_getLength(asset);
    const void* buffer = AAsset_getBuffer(asset);
    membuf sbuf((char*)buffer, (char*)buffer + sizeof(char) * length);
    std::istream in(&sbuf);
    if (in) {
      while (std::getline(in, line)) {
        LOGDEBUG("Got line: " + line);
#else
    std::ifstream file(fileLocation.c_str());

    if (file.is_open()) {
      while (getline(file, line)) {
#endif

        if (line[0] == 'v' || line[0] == 'f') {
          std::vector<std::string> tokens;

          // Wavefront file
          getTokens(line, ' ', tokens);

          int idx = 0;

          if (line[0] == 'v') {
            // get vertex
            std::vector<float> v;

            for (size_t tokenIdx = 0, tokenCount = tokens.size();
              tokenIdx < tokenCount; ++tokenIdx) {
              std::string t = tokens[tokenIdx];
              if (idx > 0)   // The first token is the vertex indicator
              {
                v.push_back(static_cast<float>(atof(t.c_str())));
              }
              ++idx;
            }
            v.push_back(1.0f); // w component
            vertices.push_back(v);
          }
          else {
            // get vertex index
            std::vector<unsigned int> v;

            for (size_t tokenIdx = 0, tokenCount = tokens.size();
              tokenIdx < tokenCount; ++tokenIdx) {
              std::string t = tokens[tokenIdx];
              if (idx > 0)   // The first token is face indicator
              {
                v.push_back((unsigned int)atoi(t.c_str()));
              }
              ++idx;
            }
            facesVertexIndexes.push_back(v);
          }
        }
      }
#ifdef __ANDROID__
      AAsset_close(asset);
#else
      file.close();
#endif

      // Correct indices. OpenGL indices are 0 based. Wavefront indices start
      // from 1 and the numbering continues for multiple objects.

      for (auto& vi : facesVertexIndexes) {
        for (auto& vi2 : vi) {
          --vi2;
        }
      }

      numBoxes = (int)(facesVertexIndexes.size() / 6);
      LOGINFO("Loaded " + std::to_string(numBoxes) + " bounding boxes.");

      triangulate();
      calcExtremes();
    }
    else
      throw std::runtime_error(
        "Could not open file " + fileLocation);

  }

  bool BoundingBoxSet::contains(const glm::vec3 point,
    const glm::vec3 thisOffset,
    const glm::vec3 thisRotation) const {

    bool collides = false;
    glm::mat4 reverseRotationMatrix =
      glm::rotate(glm::mat4x4(1.0f), -thisRotation.y, glm::vec3(0.0f, 1.0f, 0.0f)) *
      glm::rotate(glm::mat4x4(1.0f), -thisRotation.x, glm::vec3(1.0f, 0.0f, 0.0f)) *
      glm::rotate(glm::mat4x4(1.0f), -thisRotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

    glm::vec4 pointInBoxSpace = glm::vec4(point, 1.0f) -
      glm::vec4(thisOffset, 0.0f);

    pointInBoxSpace = reverseRotationMatrix * pointInBoxSpace;

    for (auto& ext : boxExtremes) {

      if (pointInBoxSpace.x > ext.minX && pointInBoxSpace.x < ext.maxX &&
        pointInBoxSpace.y > ext.minY && pointInBoxSpace.y < ext.maxY &&
        pointInBoxSpace.z > ext.minZ && pointInBoxSpace.z < ext.maxZ) {

        collides = true;
        break;
      }

    }
    return collides;
  }

  bool BoundingBoxSet::containsCorners(const BoundingBoxSet otherBoxSet,
    const glm::vec3 thisOffset,
    const glm::vec3 thisRotation,
    const glm::vec3 otherOffset,
    const glm::vec3 otherRotation) const {
    bool collides = false;

    glm::mat4 otherRotationMatrix =
      glm::rotate(glm::mat4x4(1.0f), otherRotation.y, glm::vec3(0.0f, 1.0f, 0.0f)) *
      glm::rotate(glm::mat4x4(1.0f), otherRotation.x, glm::vec3(1.0f, 0.0f, 0.0f)) *
      glm::rotate(glm::mat4x4(1.0f), otherRotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

    for (auto vertex = otherBoxSet.vertices.begin();
      vertex != otherBoxSet.vertices.end(); ++vertex) {

      glm::vec4 otherCoords(vertex->at(0), vertex->at(1), vertex->at(2), 1.0f);
      glm::vec4 rotatedOtherCoords;

      rotatedOtherCoords = otherRotationMatrix * otherCoords;

      rotatedOtherCoords.x += otherOffset.x;
      rotatedOtherCoords.y += otherOffset.y;
      rotatedOtherCoords.z += otherOffset.z;

      if (contains(glm::vec3(rotatedOtherCoords.x, rotatedOtherCoords.y,
        rotatedOtherCoords.z),
        thisOffset, thisRotation)) {

        collides = true;
        break;
      }
    }
    return collides;
  }

  void BoundingBoxSet::triangulate()
  {
    if (facesVertexIndexesTriangulated.empty()) {

      for (auto& x : facesVertexIndexes) {
        facesVertexIndexesTriangulated.push_back(std::vector<unsigned int> {x.at(0), x.at(1), x.at(2)});
        facesVertexIndexesTriangulated.push_back(std::vector<unsigned int> {x.at(2), x.at(3), x.at(0)});
      }
    }
  }

  void BoundingBoxSet::calcExtremes() {
    boxExtremes.clear();
    for (size_t idx = 0; idx < numBoxes; ++idx) {

      extremes ext;

      glm::vec4 coords(vertices[static_cast<unsigned int>(idx * 8)][0],
        vertices[static_cast<unsigned int>(idx * 8)][1],
        vertices[static_cast<unsigned int>(idx * 8)][2], 1);

      ext.minX = coords.x;
      ext.maxX = coords.x;
      ext.minY = coords.y;
      ext.maxY = coords.y;
      ext.minZ = coords.z;
      ext.maxZ = coords.z;

      for (size_t checkidx = idx * 8; checkidx < (idx + 1) * 8; ++checkidx) {

        coords = glm::vec4(vertices[static_cast<unsigned int>(checkidx)][0],
          vertices[static_cast<unsigned int>(checkidx)][1],
          vertices[static_cast<unsigned int>(checkidx)][2], 1);

        if (coords.x < ext.minX)
          ext.minX = coords.x;
        if (coords.x > ext.maxX)
          ext.maxX = coords.x;
        if (coords.y < ext.minY)
          ext.minY = coords.y;
        if (coords.y > ext.maxY)
          ext.maxY = coords.y;
        if (coords.z < ext.minZ)
          ext.minZ = coords.z;
        if (coords.z > ext.maxZ)
          ext.maxZ = coords.z;
      }

      boxExtremes.push_back(ext);
    }
  }

  int BoundingBoxSet::getNumBoxes() const {
    return numBoxes;
  }

  std::vector<Model> BoundingBoxSet::getModels() {

    std::vector<Model> models;

    for (auto boxIdx = 0; boxIdx < getNumBoxes(); ++boxIdx) {
      Model m;

      for (auto vertexIdx = 0; vertexIdx < 8; ++vertexIdx) {
        for (auto vertexComponent : vertices[boxIdx * (size_t)8 + vertexIdx]) {
          m.vertexData.push_back(vertexComponent);
          m.vertexDataByteSize += sizeof(float);
        }
      }

      for (auto faceIndexIdx = 0; faceIndexIdx < 12; ++faceIndexIdx) {
        for (auto vertexIndex : facesVertexIndexesTriangulated[boxIdx * (size_t)12 + faceIndexIdx]) {
          m.indexData.push_back(vertexIndex - boxIdx * 8);
          m.indexDataByteSize += sizeof(uint32_t);
        }
      }

      m.normalsData = std::vector<float>(24); // 8 vertex coords * 3 components per normal
      m.normalsDataByteSize = 24 * sizeof(float);

      m.textureCoordsData = std::vector<float>(16); // 8 vertex coords * 2 components per texture coord
      m.textureCoordsDataByteSize = 16 * sizeof(float);

      models.push_back(m);
    }

    return models;
  }

  void BoundingBoxSet::generateBoxesFromExtremes() {
    vertices.clear();
    facesVertexIndexes.clear();
    facesVertexIndexesTriangulated.clear();
    numBoxes = 0;
    uint32_t base = 0;
    std::vector<float> v;
    std::vector<unsigned int> i;
    for (auto& ext : boxExtremes) {
      
      v = { ext.minX, ext.minY, ext.maxZ, 1.0f };
      vertices.push_back(v);
      v = { ext.minX, ext.minY, ext.minZ, 1.0f };
      vertices.push_back(v);
      v = { ext.maxX, ext.minY, ext.minZ, 1.0f };
      vertices.push_back(v);
      v = { ext.maxX, ext.minY, ext.maxZ, 1.0f };
      vertices.push_back(v);
      v = { ext.minX, ext.maxY, ext.maxZ, 1.0f };
      vertices.push_back(v);
      v = { ext.minX, ext.maxY, ext.minZ, 1.0f };
      vertices.push_back(v);
      v = { ext.maxX, ext.maxY, ext.minZ, 1.0f };
      vertices.push_back(v);
      v = { ext.maxX, ext.maxY, ext.maxZ, 1.0f };
      vertices.push_back(v);

      i = { base + 4, base + 5, base + 1 , base + 0 };
      facesVertexIndexes.push_back(i);
      i = { base + 5, base + 6, base + 2 ,base + 1 };
      facesVertexIndexes.push_back(i);
      i = { base + 6, base + 7, base + 3 ,base + 2 };
      facesVertexIndexes.push_back(i);
      i = { base + 7, base + 4, base + 0 ,base + 3 };
      facesVertexIndexes.push_back(i);
      i = { base + 0, base + 1, base + 2 ,base + 3 };
      facesVertexIndexes.push_back(i);
      i = { base + 7, base + 6, base + 5 ,base + 4 };
      facesVertexIndexes.push_back(i);

      base += 8;
      ++numBoxes;
    }
    triangulate();
  }

  void BoundingBoxSet::generateExtremes(std::vector<float>& vertexData) {
    vertices.clear();
    facesVertexIndexes.clear();
    facesVertexIndexesTriangulated.clear();
    boxExtremes.clear();
    extremes ex = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
    
    for (uint32_t idx = 0; idx < vertexData.size(); ++idx) {
      if (idx % 4 == 0) {
        if (vertexData[idx] < ex.minX) ex.minX = vertexData[idx];
        else if (vertexData[idx] > ex.maxX) ex.maxX = vertexData[idx];
      }
      else if (idx % 4 == 1) {
        if (vertexData[idx] < ex.minY) ex.minY = vertexData[idx];
        else if (vertexData[idx] > ex.maxY) ex.maxY = vertexData[idx];
      }
      else if (idx % 4 == 2) {
        if (vertexData[idx] < ex.minZ) ex.minZ = vertexData[idx];
        else if (vertexData[idx] > ex.maxZ) ex.maxZ = vertexData[idx];
      }
    }

    boxExtremes.push_back(ex);
    generateBoxesFromExtremes();

  }

}
