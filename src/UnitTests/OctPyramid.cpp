/*
 * OctPyramid.cpp
 *
 *  Created on: 9 Oct 2021
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */
#include "OctPyramid.hpp"

glm::vec3 OctPyramid::normalBetween(uint64_t triangle1, uint64_t triangle2) {

  // First triangle 
  auto edge1 = indexData[3 * triangle1 + 1];
  auto edge2 = indexData[3 * triangle1 + 2];

  auto cross1 = glm::cross(glm::vec3(vertexData[static_cast<uint64_t>(edge1) * 4], vertexData[static_cast<uint64_t>(edge1) * 4 + 1] - height, vertexData[static_cast<uint64_t>(edge1) * 4 + 2]),
    glm::vec3(vertexData[static_cast<uint64_t>(edge2) * 4], vertexData[static_cast<uint64_t>(edge2) * 4 + 1] - height, vertexData[static_cast<uint64_t>(edge2) * 4 + 2]));

  // Second triangle
  auto edge3 = indexData[3 * triangle2 + 1];
  auto edge4 = indexData[3 * triangle2 + 2];

  auto cross2 = glm::cross(glm::vec3(vertexData[static_cast<uint64_t>(edge3) * 4], vertexData[static_cast<uint64_t>(edge3) * 4 + 1] - height, vertexData[static_cast<uint64_t>(edge3) * 4 + 2]),
    glm::vec3(vertexData[static_cast<uint64_t>(edge4) * 4], vertexData[static_cast<uint64_t>(edge4) * 4 + 1] - height, vertexData[static_cast<uint64_t>(edge4) * 4 + 2]));

  // Add cross products
  auto normal = cross1 + cross2;

  // Normalise
  normal /= std::sqrt(std::pow(normal.x, 2) + std::pow(normal.y, 2) + std::pow(normal.z, 2));

  return normal;
}

OctPyramid::OctPyramid(const float height, const float radius) {
  this->height = height;
  this->radius = radius;

  const float edgeLengthFactor = static_cast<float>(std::sin(3.14 / 8));
  
  // Top vertex and normal
  this->vertexData.insert(vertexData.end(), { 0.0f, height, 0.0f, 1.0f });
  this->normalsData.insert(normalsData.end(), { 0.0f, 1.0f, 0.0f });

  // First vertex at base
  glm::vec3 pos(radius, 0.0f, -2 * radius * edgeLengthFactor);
  this->vertexData.insert(vertexData.end(), { pos.x, pos.y, pos.z, 1.0f });

  glm::vec3 dir(0.0f, 0.0f, -1.0f);

  // From third vertex on...
  for (uint32_t i = 0; i < 8; ++i) {
    
    // Add vertex
    dir = glm::vec3(glm::rotate(glm::mat4x4(1.0f), 0.785f, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(dir, 1.0f));
    pos += 2 * radius * edgeLengthFactor * dir;
    this->vertexData.insert(vertexData.end(), { pos.x, pos.y, pos.z, 1.0f });
    
    // Add triangle indices
    this->indexData.insert(indexData.end(), { 0, i + 1, i + 2 });

    // Add normal (beginning normal left out to be calculated using 
    // the first and last triangle)
    if (i > 0) { 
      auto nm = normalBetween(i - 1, i);
      this->normalsData.insert(normalsData.end(), { nm.x, nm.y, nm.z });
    }
  }
  // One more triangle to close rounding error gap
  this->indexData.insert(indexData.end(), { 0, 9, 1 });
  auto nm = normalBetween(7, 8);
  this->normalsData.insert(normalsData.end(), { nm.x, nm.y, nm.z });

  // insert first-vertex / last-vertex normal
  nm = normalBetween(8, 0);
  this->normalsData.insert(normalsData.begin() + 3, { nm.x, nm.y, nm.z });

  this->vertexDataByteSize = static_cast<uint32_t>(this->vertexData.size() * sizeof(float));
  this->indexDataByteSize = static_cast<uint32_t>(this->indexData.size() * sizeof(float));
  this->normalsDataByteSize = static_cast<uint32_t>(this->normalsData.size() * sizeof(float));

}