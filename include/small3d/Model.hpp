/**
 * @file Model.hpp
 * @brief A 3D model class
 *
 * Created on: 2014/10/18
 *     Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */
#pragma once

#include <string>
#include <vector>

#ifndef SMALL3D_OPENGL
#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#endif

#include "GlbFile.hpp"

namespace small3d {
  /**
   * @struct	Model
   *
   * @brief	A 3D model. It can be loaded from a Wavefront file.
   *            Such a file can be exported from Blender for example (see blender.org).
   *            From its menu, select File > Export > Wavefront (.obj). Then from the
   *            "Export OBJ" menu, only select "Write Normals", "Triangulate Faces" 
   *            and "Keep Vertex Order".
   *            The 3D model can also be constructed procedurally by code, by inserting
   *            values to the appropriate member variables.
   */

  struct Model {

  public:

    struct Joint {
      uint32_t id;
      std::string name;
      glm::mat4 inverseBindMatrix;
      glm::quat rotation;
      glm::vec3 scale;
      glm::vec3 translation;
      std::vector<uint32_t> children;
    };

    static const uint32_t MAX_JOINTS_SUPPORTED = 32;

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

#ifndef SMALL3D_OPENGL
    /**
     * @brief Is the model already in GPU memory?
     *        (Vulkan-specific, avoid direct manipulation)
     */
    bool alreadyInGPU = false;

    /**
     * @brief Buffer containing vertex positions
     *        (Vulkan-specific, avoid direct manipulation)
     */
    VkBuffer positionBuffer;

    /**
     * @brief Memory for buffer containing vertex positions
     *        (Vulkan-specific, avoid direct manipulation)
     */
    VkDeviceMemory positionBufferMemory;

    /**
     * @brief Index buffer
     *        (Vulkan-specific, avoid direct manipulation)
     */
    VkBuffer indexBuffer;

    /**
     * @brief Memory for index buffer
     *        (Vulkan-specific, avoid direct manipulation)
     */
    VkDeviceMemory indexBufferMemory;

    /**
     * @brief Normals buffer
     *        (Vulkan-specific, avoid direct manipulation)
     */
    VkBuffer normalsBuffer;

    /**
     * @brief Memory for normals buffer
     *        (Vulkan-specific, avoid direct manipulation)
     */
    VkDeviceMemory normalsBufferMemory;

    /**
     * @brief UV coordinates buffer
     *        (Vulkan-specific, avoid direct manipulation)
     */
    VkBuffer uvBuffer;

    /**
     * @brief Memory for UV coordinates buffer
     *        (Vulkan-specific, avoid direct manipulation)
     */
    VkDeviceMemory uvBufferMemory;

    /**
     * @brief Index of model orientation in the orientation dynamic uniform 
     *        buffer
     *        (Vulkan-specific, avoid direct manipulation)
     */
    uint32_t placementMemIndex = 0;

    /**
     * @brief Index of model colour in the colour dynamic uniform buffer
     *        (Vulkan-specific, avoid direct manipulation)
     */
    uint32_t colourMemIndex = 0;

    /**
     * @brief Joints buffer
     */
    VkBuffer jointBuffer;

    /**
    * @brief Memory for joints buffer
    *        (Vulkan-specific, avoid direct manipulation)
    */
    VkDeviceMemory jointBufferMemory;

    /**
     * @brief Joint weights buffer
     */
    VkBuffer weightBuffer;

    /**
    * @brief Memory for weights buffer
    *        (Vulkan-specific, avoid direct manipulation)
    */
    VkDeviceMemory weightBufferMemory;
#else

    /**
     * @brief Position buffer object id. It is suggested not to manipulate
     *        this directly.
     */
    uint32_t positionBufferObjectId = 0;

    /**
     * @brief Index buffer object id. It is suggested not to manipulate
     *        this directly.
     */
    uint32_t indexBufferObjectId = 0;

    /**
     * @brief Normals buffer object id. It is suggested not to manipulate
     *        this directly.
     */
    uint32_t normalsBufferObjectId = 0;

    /**
     * @brief UV buffer object id. It is suggested not to manipulate this
     *        directly.
     */
    uint32_t uvBufferObjectId = 0;
    
#endif

    /**
     * @brief The vertex data. This is an array, which is to be treated as a 4
     *        column table, holding the x, y, z values in each column. The
     *        fourth column is there to assist in matrix operations.
     */
    std::vector<float> vertexData;

    /**
     * @brief Size of the vertex data, in bytes.
     */
    uint32_t vertexDataByteSize = 0;

    /**
     * @brief 3 column table. Each element refers to a "row" in the vertex data
     *        table. Each "row" in the index data table forms a triangle.
     *
     */
    std::vector<uint32_t> indexData;

    /**
     * @brief Size of the index data, in bytes
     */
    uint32_t indexDataByteSize = 0;

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
    uint32_t normalsDataByteSize = 0;

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
    uint32_t textureCoordsDataByteSize = 0;

    /**
     * @brief Array, to be treated as a 4 column table. Each "row" potentially 
     *        contains up to 4 joints that can influence each vertex. 
     */
    std::vector<uint8_t> jointData;

    /**
     * @brief Size of the joint data, in bytes.
     */ 
    uint32_t jointDataByteSize = 0;

    /**
     * @brief Array, to be treated as a 4 column table. Each "row" contains the
     *        weights by which each of the corresponding joints affect each vertex.
     */
    std::vector<float> weightData;

    /**
     * @brief Size of the weight data, in bytes.
     */
    uint32_t weightDataByteSize = 0;

    /**
     * @brief The model's root joint;
     */
    std::vector<Joint> joints;


#ifndef SMALL3D_OPENGL
    
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
    
#endif
    
    /**
     * @brief Default constructor
     *
     */
    Model();

    /**
     * @brief Constructor that loads model from a Wavefront file
     * @param fileLocation Location of the Wavefront file from which to load the
     *                     model.
     */
    Model(const std::string fileLocation);


    /**
     * @brief Constructor that loads model from a GLB file
     * @param fileLocation Location of the GLB file from which to load the
     *                     model.
     * @param meshName The name of the mesh in the GLB file which will be loaded
     *                 as the model.
     */
    Model(const std::string& fileLocation, const std::string& meshName);

  };
}
