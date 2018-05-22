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

#include <GL/glew.h>

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

    /**
     * @brief OpenGL position buffer object id. It is suggested not to manipulate
     *        this directly.
     */
    GLuint positionBufferObjectId = 0;

    /**
     * @brief OpenGL index buffer object id. It is suggested not to manipulate
     *        this directly.
     */
    GLuint indexBufferObjectId = 0;

    /**
     * @brief OpenGL normals buffer object id. It is suggested not to manipulate
     *        this directly.
     */
    GLuint normalsBufferObjectId = 0;

    /**
     * @brief OpenGL UV buffer object id. It is suggested not to manipulate this
     *        directly.
     */
    GLuint uvBufferObjectId = 0;
    
    /**
     * @brief The vertex data. This is an array, which is to be treated as a 4
     *        column table, holding the x, y, z values in each column. The
     *        fourth column is there to assist in matrix operations.
     */
    std::vector<float> vertexData;

    /**
     * @brief Size of the vertex data, in bytes.
     */
    int vertexDataByteSize;

    /**
     * @brief 3 column table. Each element refers to a "row" in the vertex data
     *        table. Each "row" in the index data table forms a triangle.
     *
     */
    std::vector<unsigned int> indexData;

    /**
     * @brief Size of the index data, in bytes
     */
    int indexDataByteSize;

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
    int normalsDataByteSize;

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
    int textureCoordsDataByteSize;

    /**
     * @brief constructor
     * @param fileLocation Location of the Wavefront file from which to load the
     *                     model.
     */
    Model(const std::string fileLocation);

  };
}
