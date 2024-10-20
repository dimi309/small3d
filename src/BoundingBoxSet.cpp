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
#include "BasePath.hpp"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

namespace small3d {

  BoundingBoxSet::BoundingBoxSet() {

  }

  BoundingBoxSet::BoundingBoxSet(const std::vector<float>& vertexData,
    const glm::vec3& scale, uint32_t subdivisions) {
    generateExtremes(vertexData, scale, subdivisions);

  }

  bool BoundingBoxSet::contains(const glm::vec3 point,
    const glm::vec3 thisOffset,
    const glm::mat4x4 thisRotation) const {

    bool doesContain = false;
    glm::mat4 reverseRotationMatrix = glm::inverse(thisRotation);

    glm::vec4 pointInBoxSpace = glm::vec4(point, 1.0f) -
      glm::vec4(thisOffset, 0.0f);

    pointInBoxSpace = reverseRotationMatrix * pointInBoxSpace;


    if (std::any_of(boxExtremes.begin(), boxExtremes.end(), [&pointInBoxSpace](const auto& ex) {return pointInBoxSpace.x > ex.minX && pointInBoxSpace.x < ex.maxX &&
      pointInBoxSpace.y > ex.minY && pointInBoxSpace.y < ex.maxY &&
      pointInBoxSpace.z > ex.minZ && pointInBoxSpace.z < ex.maxZ; })) {
      doesContain = true;
    }

    return doesContain;
  }

  bool BoundingBoxSet::containsCorners(const BoundingBoxSet& otherBoxSet,
    const glm::vec3 thisOffset,
    const glm::mat4x4 thisRotation,
    const glm::vec3 otherOffset,
    const glm::mat4x4 otherRotation) const {
    bool collides = false;

    for (auto vertex = otherBoxSet.vertices.begin();
      vertex != otherBoxSet.vertices.end(); ++vertex) {

      glm::vec4 otherCoords(vertex->at(0), vertex->at(1), vertex->at(2), 1.0f);
      glm::vec4 rotatedOtherCoords;

      rotatedOtherCoords = otherRotation * otherCoords;

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

  void BoundingBoxSet::generateBoxesFromExtremes() {
    vertices.clear();
    facesVertexIndexes.clear();
    facesVertexIndexesTriangulated.clear();
    numBoxes = 0;
    uint32_t base = 0;
     
    for (auto& ext : boxExtremes) {

      vertices.emplace_back(std::vector<float>{ ext.minX, ext.minY, ext.maxZ, 1.0f });
      vertices.emplace_back(std::vector<float>{ ext.minX, ext.minY, ext.minZ, 1.0f });
      vertices.emplace_back(std::vector<float>{ ext.maxX, ext.minY, ext.minZ, 1.0f });
      vertices.emplace_back(std::vector<float>{ ext.maxX, ext.minY, ext.maxZ, 1.0f });
      vertices.emplace_back(std::vector<float>{ ext.minX, ext.maxY, ext.maxZ, 1.0f });
      vertices.emplace_back(std::vector<float>{ ext.minX, ext.maxY, ext.minZ, 1.0f });
      vertices.emplace_back(std::vector<float>{ ext.maxX, ext.maxY, ext.minZ, 1.0f });
      vertices.emplace_back(std::vector<float>{ ext.maxX, ext.maxY, ext.maxZ, 1.0f });


      facesVertexIndexes.emplace_back(std::vector<unsigned int>{ base + 4, base + 5, base + 1, base + 0 });
      facesVertexIndexes.emplace_back(std::vector<unsigned int>{ base + 5, base + 6, base + 2 ,base + 1 });
      facesVertexIndexes.emplace_back(std::vector<unsigned int>{ base + 6, base + 7, base + 3 ,base + 2 });
      facesVertexIndexes.emplace_back(std::vector<unsigned int>{ base + 7, base + 4, base + 0 ,base + 3 });
      facesVertexIndexes.emplace_back(std::vector<unsigned int>{ base + 0, base + 1, base + 2 ,base + 3 });
      facesVertexIndexes.emplace_back(std::vector<unsigned int>{ base + 7, base + 6, base + 5 ,base + 4 });

      base += 8;
      ++numBoxes;
    }
    triangulate();
  }

  void BoundingBoxSet::generateExtremes(const std::vector<float>& vertexData, const glm::vec3& scale, uint32_t subdivisions) {
    vertices.clear();
    facesVertexIndexes.clear();
    facesVertexIndexesTriangulated.clear();
    boxExtremes.clear();
    extremes ex;

    glm::vec4 vertex(0.0f, 0.0f, 0.0f, 0.0f);

    for (uint32_t idx = 0; idx < vertexData.size(); ++idx) {
      if (idx % 4 == 0) {
        vertex.x = vertexData[idx];
      }
      else if (idx % 4 == 1) {
        vertex.y = vertexData[idx];
      }
      else if (idx % 4 == 2) {
        vertex.z = vertexData[idx];
      }
      else if (idx % 4 == 3) {
        vertex.w = vertexData[idx];
        vertex = glm::scale(glm::mat4x4(1.0f), scale) * vertex;

        if (vertex.x < ex.minX) ex.minX = vertex.x;
        else if (vertex.x > ex.maxX) ex.maxX = vertex.x;

        if (vertex.y < ex.minY) ex.minY = vertex.y;
        else if (vertex.y > ex.maxY) ex.maxY = vertex.y;

        if (vertex.z < ex.minZ) ex.minZ = vertex.z;
        else if (vertex.z > ex.maxZ) ex.maxZ = vertex.z;
      }
    }

    boxExtremes.push_back(ex);

    for (uint32_t idx = 0; idx < subdivisions; ++idx) {
      generateSubExtremes(vertexData, scale);
    }
    generateBoxesFromExtremes();
  }

  void BoundingBoxSet::generateSubExtremes(const std::vector<float>& vertexData, const glm::vec3& scale) {

    // Move all extremes to a temporary buffer
    std::vector<extremes> extBuffer;

    std::move(boxExtremes.begin(), boxExtremes.end(), std::back_inserter(extBuffer));
    boxExtremes.clear();
    // Break each extreme into 8
    for (const auto& ext : extBuffer) {

      float xSplit = ext.minX + (ext.maxX - ext.minX) / 2.0f;
      float zSplit = ext.minZ + (ext.maxZ - ext.minZ) / 2.0f;
      float ySplit = ext.minY + (ext.maxY - ext.minY) / 2.0f;

      std::vector<extremes> newExtremes;
      newExtremes.resize(8);

      newExtremes[0].minX = ext.minX;
      newExtremes[0].maxX = xSplit;
      newExtremes[0].minZ = ext.minZ;
      newExtremes[0].maxZ = zSplit;
      newExtremes[0].minY = ext.minY;
      newExtremes[0].maxY = ySplit;

      newExtremes[1].minX = ext.minX;
      newExtremes[1].maxX = xSplit;
      newExtremes[1].minZ = zSplit;
      newExtremes[1].maxZ = ext.maxZ;
      newExtremes[1].minY = ext.minY;
      newExtremes[1].maxY = ySplit;

      newExtremes[2].minX = xSplit;
      newExtremes[2].maxX = ext.maxX;
      newExtremes[2].minZ = zSplit;
      newExtremes[2].maxZ = ext.maxZ;
      newExtremes[2].minY = ext.minY;
      newExtremes[2].maxY = ySplit;

      newExtremes[3].minX = xSplit;
      newExtremes[3].maxX = ext.maxX;
      newExtremes[3].minZ = ext.minZ;
      newExtremes[3].maxZ = zSplit;
      newExtremes[3].minY = ext.minY;
      newExtremes[3].maxY = ySplit;

      newExtremes[4].minX = ext.minX;
      newExtremes[4].maxX = xSplit;
      newExtremes[4].minZ = ext.minZ;
      newExtremes[4].maxZ = zSplit;
      newExtremes[4].minY = ySplit;
      newExtremes[4].maxY = ext.maxY;

      newExtremes[5].minX = ext.minX;
      newExtremes[5].maxX = xSplit;
      newExtremes[5].minZ = zSplit;
      newExtremes[5].maxZ = ext.maxZ;
      newExtremes[5].minY = ySplit;
      newExtremes[5].maxY = ext.maxY;

      newExtremes[6].minX = xSplit;
      newExtremes[6].maxX = ext.maxX;
      newExtremes[6].minZ = zSplit;
      newExtremes[6].maxZ = ext.maxZ;
      newExtremes[6].minY = ySplit;
      newExtremes[6].maxY = ext.maxY;

      newExtremes[7].minX = xSplit;
      newExtremes[7].maxX = ext.maxX;
      newExtremes[7].minZ = ext.minZ;
      newExtremes[7].maxZ = zSplit;
      newExtremes[7].minY = ySplit;
      newExtremes[7].maxY = ext.maxY;

      glm::vec4 point;

      // From the newly formed extremes keep only the ones that
      // contain one of the model's vertices

      for (uint32_t idx = 0; idx < vertexData.size(); ++idx) {
        if (idx % 4 == 0) {
          point.x = vertexData[idx];
        }
        else if (idx % 4 == 1) {
          point.y = vertexData[idx];
        }
        else if (idx % 4 == 2) {
          point.z = vertexData[idx];
        }
        else if (idx % 4 == 3) {
          point.w = vertexData[idx];

          point = glm::scale(glm::mat4x4(1.0f), scale) * point;

          for (auto& ex : newExtremes) {

            if (point.x >= ex.minX && point.x <= ex.maxX &&
              point.y >= ex.minY && point.y <= ex.maxY &&
              point.z >= ex.minZ && point.z <= ex.maxZ) ex.tagged = true;

          }
        }
      }

      std::copy_if(newExtremes.begin(), newExtremes.end(), std::back_inserter(boxExtremes),
        [](const auto& ex) {return ex.tagged; });

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
          m.indexDataByteSize += sizeof(uint16_t);
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

}
