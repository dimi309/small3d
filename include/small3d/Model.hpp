/**
 * @file Model.hpp
 * @brief Header of the Model class
 *
 * Created on: 2014/10/18
 *     Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */
#pragma once

#include <string>
#include <vector>
#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

namespace small3d {
  /**
   * @struct	Model
   *
   * @brief	A 3D model, loaded from a Wavefront file.
   */

  struct Model {

  private:
    // Data read from .obj file
    std::vector<std::vector<float> > vertices;
    std::vector<std::vector<int> > facesVertexIndices;
    std::vector<std::vector<float> > normals;
    std::vector<std::vector<int> > facesNormalIndices;
    std::vector<std::vector<float> > textureCoords;
    std::vector<std::vector<int> > textureCoordsIndices;

    void loadVertexData();
    void loadIndexData();
    void loadNormalsData();
    void loadTextureCoordsData();

    // Make sure that no texture coordinate information is lost when the data
    // buffers get created (vertexData, indexData, normalsData and
    // textureCoordsData) by realigning the data vectors, in order to ensure
    // unique vertex - texture coordinates pairs.
    void correctDataVectors();

    void clear();

  public:

    // Vulkan-specific variables. Avoid manipulating these directly.

    bool alreadyInGPU = false;

    VkBuffer positionBuffer;
    VkDeviceMemory positionBufferMemory;

    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    VkBuffer normalsBuffer;
    VkDeviceMemory normalsBufferMemory;

    VkBuffer uvBuffer;
    VkDeviceMemory uvBufferMemory;

    uint32_t orientationMemIndex = 0;

    uint32_t colourMemIndex = 0;

    // End of Vulkan-specific variables.
        
    /**
     * @brief The vertex data. This is an array, which is to be treated as a 4
     *        column table, holding the x, y, z values in each column. The
     *        fourth column is there to assist in matrix operations.
     */
    std::vector<float> vertexData;

    /**
     * @brief Size of the vertex data, in bytes.
     */
    int vertexDataByteSize = 0;

    /**
     * @brief 3 column table. Each element refers to a "row" in the vertex data
     *        table. Each "row" in the index data table forms a triangle.
     *
     */
    std::vector<uint32_t> indexData;

    /**
     * @brief Size of the index data, in bytes
     */
    int indexDataByteSize = 0;

    /**
     * @brief Array, to be treated as a 3 column table. Each "row" contains the
     *        x, y and z components of the vector representing the normal of a
     *        vertex. The position of the "row" in the array is the same as the
     *        position of the corresponding vertex "row" in the vertexData array.
     */
    std::vector<float> normalsData;

    /**
     * @brief Size of the normals data, in bytes.
     */
    int normalsDataByteSize = 0;

    /**
     * @brief Array, to be treated as a 2 column table. Each "row" contains the
     *        x and y components of the pixel coordinates on the model's texture
     *        image for the vertex in the corresponding "row" of the vertex data
     *        "table"
     */
    std::vector<float> textureCoordsData;

    /**
     * @brief Size of the texture coordinates data, in bytes.
     */
    int textureCoordsDataByteSize = 0;

    /**
    * @brief Name of the texture the model will be rendered with. The texture has to
    *        have been previously generated with Renderer.generateTexture().
    */
    std::string textureName = "";

    /**
     * @brief True if the model is to be rendered with perspective, False for orthographic
     *        rendering
     */
    bool perspective = false;

    
    /**
     * @brief Default constructor
     *
     */
    Model();

    /**
     * @brief Constructor that loads model from file
     * @param fileLocation Location of the Wavefront file from which to load the
     *                     model.
     */
    Model(const std::string fileLocation);

  };
}
