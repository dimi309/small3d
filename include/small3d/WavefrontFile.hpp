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
#include "BoundingBoxSet.hpp"

namespace small3d {
  class WavefrontFile {
  private:

    std::string fileLocation = "";

    // Does this file only contain triangles?
    bool onlyTriangles = true;

    // Data read from .obj file
    std::vector<std::vector<float> > vertices;
    std::vector<std::vector<uint32_t> > facesVertexIndices;
    std::vector<std::vector<float> > normals;
    std::vector<std::vector<uint32_t> > facesNormalIndices;
    std::vector<std::vector<float> > textureCoords;
    std::vector<std::vector<uint32_t> > textureCoordsIndices;

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
    /**
     * @brief Constructor
     * @param fileLocation Path to wavefront file
     */
    WavefrontFile(const std::string& fileLocation);

    /**
     * @brief Load data from the Wavefront file into a Model
     * @param The model to load the data to
     */
    void load(Model& model);

    /**
     * @brief Load data from the Wavefront file into a BoundingBoxSet
     * @param The BoundingBoxSet to load the data to
     */
    void load(BoundingBoxSet& boundingBoxSet);
  };
}
