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

namespace small3d {
  
  /**
   * Constructor
   */
  
  BoundingBoxSet::BoundingBoxSet(const std::string fileLocation) {
    initLogger();
    vertices.clear();
    facesVertexIndexes.clear();
    numBoxes = 0;
    
    if (fileLocation != "") this->loadFromFile(fileLocation);
    
  }
  
  void BoundingBoxSet::loadFromFile(std::string fileLocation) {
    if (vertices.size() != 0) {
      throw std::runtime_error("Illegal attempt to reload bounding boxes. "
			       "Please use another object.");
    }
    std::ifstream file(fileLocation.c_str());
    std::string line;
    if (file.is_open()) {
      while (getline(file, line)) {
        if (line[0] == 'v' || line[0] == 'f') {
          std::vector<std::string> tokens;
          
          // Wavefront file
          getTokens(line, ' ', tokens);
          
          int idx = 0;
          
          if (line[0] == 'v') {
            // get vertex
            std::vector<float> v;
            
            for (size_t tokenIdx = 0, tokenCount= tokens.size();
		 tokenIdx < tokenCount; ++tokenIdx) {
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
            std::vector<unsigned int> v;
            
            for (size_t tokenIdx = 0, tokenCount = tokens.size();
		 tokenIdx < tokenCount; ++tokenIdx) {
              std::string t = tokens[tokenIdx];
              if (idx > 0)   // The first token is face indicator
              {
                v.push_back((unsigned int) atoi(t.c_str()));
              }
              ++idx;
            }
            facesVertexIndexes.push_back(v);
          }
        }
      }
      file.close();
      numBoxes = (int) (facesVertexIndexes.size() / 6);
      
      // Correct indices. OpenGL indices are 0 based. Wavefront indices start
      // from 1 and the numbering continues for multiple objects.
      
      for (int idx = 0; idx < numBoxes; ++idx) {
        for(int idx2 = 0; idx2 < 6; ++idx2) {
          for(int idx3 = 0; idx3 < 4; ++idx3) {
            facesVertexIndexes[6 * idx + idx2][idx3] -= 1 + 8 * idx;
          }
        }
      }
      
      LOGINFO("Loaded " + intToStr(numBoxes) + " bounding boxes.");
    }
    else
      throw std::runtime_error(
                      "Could not open file " + fileLocation);
    
  }
  
  bool BoundingBoxSet::collidesWith(const glm::vec3 point,
				    const glm::vec3 thisOffset,
				    const glm::vec3 thisRotation) const {
    bool collides = false;
    glm::mat4 rotationMatrix = 
      glm::rotate(
        glm::rotate(
          glm::rotate(glm::mat4x4(1.0f), -thisRotation.y,
		      glm::vec3(0.0f, -1.0f, 0.0f)),
	  -thisRotation.x,
	  glm::vec3(-1.0f, 0.0f, 0.0f)), -thisRotation.z,
	glm::vec3(0.0f, 0.0f, -1.0f)
      );
    
    glm::vec4 pointInBoxSpace = glm::vec4(point, 1.0f) -
      glm::vec4(thisOffset, 0.0f);
    
    pointInBoxSpace = rotationMatrix * pointInBoxSpace;
    
    for (int idx = 0; idx < numBoxes; ++idx) {
      float minZ, maxZ, minX, maxX, minY, maxY;
      
      glm::vec4 coords(vertices[static_cast<unsigned int>(idx * 8)][0],
		       vertices[static_cast<unsigned int>(idx * 8)][1],
                       vertices[static_cast<unsigned int>(idx * 8)][2], 1);
      
      glm::vec4 rotatedCoords;
      rotatedCoords = coords;
      
      minX = rotatedCoords.x;
      maxX = rotatedCoords.x;
      minY = rotatedCoords.y;
      maxY = rotatedCoords.y;
      minZ = rotatedCoords.z;
      maxZ = rotatedCoords.z;
      
      for (int checkidx = idx * 8; checkidx < (idx + 1) * 8; ++checkidx) {
        coords = glm::vec4(vertices[static_cast<unsigned int>(checkidx)][0],
			   vertices[static_cast<unsigned int>(checkidx)][1],
                           vertices[static_cast<unsigned int>(checkidx)][2], 1);
        rotatedCoords = coords;
        
        if (rotatedCoords.x < minX)
          minX = rotatedCoords.x;
        if (rotatedCoords.x > maxX)
          maxX = rotatedCoords.x;
        if (rotatedCoords.y < minY)
          minY = rotatedCoords.y;
        if (rotatedCoords.y > maxY)
          maxY = rotatedCoords.y;
        if (rotatedCoords.z < minZ)
          minZ = rotatedCoords.z;
        if (rotatedCoords.z > maxZ)
          maxZ = rotatedCoords.z;
      }
      
      if (pointInBoxSpace.x > minX && pointInBoxSpace.x < maxX &&
          pointInBoxSpace.y > minY && pointInBoxSpace.y < maxY &&
          pointInBoxSpace.z > minZ && pointInBoxSpace.z < maxZ) {
        
        collides = true;
        break;
      }
    }
    return collides;
  }
  
  bool BoundingBoxSet::collidesWith(const BoundingBoxSet otherBoxSet,
				    const glm::vec3 thisOffset,
				    const glm::vec3 thisRotation,
				    const glm::vec3 otherOffset,
				    const glm::vec3 otherRotation) const {
    bool collides = false;
    
    glm::mat4 rotationMatrix =
      glm::rotate(
        glm::rotate(
          glm::rotate(glm::mat4x4(1.0f), otherRotation.z,
		      glm::vec3(0.0f, 0.0f, -1.0f)),
	  otherRotation.x,
	  glm::vec3(-1.0f, 0.0f, 0.0f)),
	otherRotation.y, glm::vec3(0.0f, -1.0f, 0.0f));
    
    for (auto vertex = otherBoxSet.vertices.begin();
         vertex != otherBoxSet.vertices.end(); ++vertex) {
      
      glm::vec4 otherCoords(vertex->at(0), vertex->at(1), vertex->at(2), 1.0f);
      glm::vec4 rotatedOtherCoords;
      
      rotatedOtherCoords =  rotationMatrix * otherCoords;
      
      rotatedOtherCoords.x += otherOffset.x;
      rotatedOtherCoords.y += otherOffset.y;
      rotatedOtherCoords.z += otherOffset.z;
      
      if (collidesWith(glm::vec3(rotatedOtherCoords.x, rotatedOtherCoords.y,
				 rotatedOtherCoords.z),
		       thisOffset, thisRotation)) {
        
        collides = true;
        break;
      }
    }
    
    return collides;
  }
  
  int BoundingBoxSet::getNumBoxes() const {
    return numBoxes;
  }
  
}
