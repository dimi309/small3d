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
    int numIndexes = (int) (facesVertexIndices.size() * 3);
    this->indexDataByteSize = numIndexes * sizeof(int);

    this->indexData.clear();

    for (auto face = facesVertexIndices.begin();
	 face != facesVertexIndices.end(); ++face) {

      for (int indexIdx = 0; indexIdx != 3; ++indexIdx) {
        this->indexData.push_back(face->at((unsigned long) indexIdx)
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
    if (!textureCoords.empty()) {
      // Create an array of texture coordinates components which corresponds
      // by index to the array of vertex components

      if (this->vertexData.size() == 0) {
        throw std::runtime_error("There are no vertices or vertex data has not "
				 "yet been created.");
      }

      // 2 components per vertex (a single index for vertices, normals and
      // texture coordinates is passed to OpenGL, so texture coordinates data
      // will be aligned to vertex data according to the vertex index)
      this->textureCoordsDataByteSize = (int) (2 * vertices.size() *
					       sizeof(float));

      this->textureCoordsData = std::vector<float>(2 * vertices.size());

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
	  // in a pair with a different texture coordinates index number (see above)
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


  Model::Model(const std::string fileLocation) {
    std::ifstream file(fileLocation.c_str());
    std::string line;
    if (file.is_open()) {
      clear();

      while (getline(file, line)) {
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
      file.close();

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

}
