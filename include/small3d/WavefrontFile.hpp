/**
 * @file  WavefrontFile.hpp
 * @brief Wavefront (.obj) file parser
 *
 *  Created on: 2021/02/22
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#pragma once
#include <vector>
#include "Model.hpp"

namespace small3d {
  class WavefrontFile {
  private:
    // Data read from .obj file
    std::vector<std::vector<float> > vertices;
    std::vector<std::vector<int> > facesVertexIndices;
    std::vector<std::vector<float> > normals;
    std::vector<std::vector<int> > facesNormalIndices;
    std::vector<std::vector<float> > textureCoords;
    std::vector<std::vector<int> > textureCoordsIndices;

    void loadVertexData(std::vector<float>& vertexData);
    void loadIndexData(std::vector<uint32_t>& indexData);
    void loadNormalsData(std::vector<float>& normalsData, const std::vector<float>& vertexData);
    void loadTextureCoordsData(std::vector<float>& textureCoordsData, const std::vector<float>& vertexData);

    // Make sure that no texture coordinate information is lost when the data
    // buffers get created (vertexData, indexData, normalsData and
    // textureCoordsData) by realigning the data vectors, in order to ensure
    // unique vertex - texture coordinates pairs.
    void correctDataVectors();

  public:
    WavefrontFile(const std::string& fileLocation);
    void load(Model& model);
  };
}
