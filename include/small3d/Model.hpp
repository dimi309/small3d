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
#endif

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include "Image.hpp"
#include "File.hpp"

namespace small3d {

  /**
   * @class	Model
   *
   * @brief	A 3D model. It can be loaded from a WavefrontFile or GlbFile.
   *        It can also be constructed procedurally by code, by inserting
   *        values to the appropriate member variables.
   */

  class Model {

  private:

#ifndef SMALL3D_OPENGL
    bool alreadyInGPU = false;
    VkBuffer positionBuffer = 0;
    VkDeviceMemory positionBufferMemory = 0;
    VkBuffer indexBuffer = 0;
    VkDeviceMemory indexBufferMemory = 0;
    VkBuffer normalsBuffer = 0;
    VkDeviceMemory normalsBufferMemory = 0;
    VkBuffer uvBuffer = 0;
    VkDeviceMemory uvBufferMemory = 0;
    uint32_t placementMemIndex = 0;
    uint32_t colourMemIndex = 0;
    VkBuffer jointBuffer = 0;
    VkDeviceMemory jointBufferMemory = 0;
    VkBuffer weightBuffer = 0;
    VkDeviceMemory weightBufferMemory = 0;

    bool perspective = false;

    // Name of the texture the model will be rendered with. The texture has to
    // have been previously generated with Renderer.generateTexture().
    std::string textureName = "";
#else
    uint32_t positionBufferObjectId = 0;
    uint32_t indexBufferObjectId = 0;
    uint32_t normalsBufferObjectId = 0;
    uint32_t uvBufferObjectId = 0;
    uint32_t jointBufferObjectId = 0;
    uint32_t weightBufferObjectId = 0;
#endif

    uint64_t numPoses = 0;
    uint32_t currentPose = 0;

    // Original transformation matrix (from armature/skin),
    // as read from a file
    glm::mat4 origTransformation = glm::mat4(1.0f);

    // Original rotation (from armature/skin), as read from a
    // file (in quaternion form)
    glm::quat origRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

    // Original scale, as read from a file
    glm::vec3 origScale = glm::vec3(1.0f, 1.0f, 1.0f);

    // brief Original translation, as read from a file
    glm::vec3 origTranslation = glm::vec3(0.0f, 0.0f, 0.0f);

  public:

    /**
     * @brief animation joint
     */
    struct Joint {
      uint32_t node = 0;
      std::string name;
      glm::mat4 inverseBindMatrix = glm::mat4(1.0f);
      glm::quat rotation = glm::quat(0.0f, 0.0f, 0.0f, 1.0f);
      glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
      glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.0f);
      glm::quat currRotation = glm::quat(0.0f, 0.0f, 0.0f, 1.0f);
      glm::vec3 currTranslation = glm::vec3(0.0f, 0.0f, 0.0f);
      std::vector<uint32_t> children;
      std::vector<glm::quat> rotationAnimation;
      std::vector<glm::vec3> translationAnimation;
      std::vector<glm::vec3> scaleAnimation;
      std::vector<float> animTime;
    };

    /**
     * @brief Use this to scale the model and not origScale
     */
    glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);

    /**
     * @brief Maximum number of supported joints
     *
     */
    static const uint32_t MAX_JOINTS_SUPPORTED = 16;

    /**
     * @brief Image found in the file the model was loaded from (empty image if not - check the size).
     *        It can be used to generate a texture for the model, but other textures can also be used.
     */
    std::shared_ptr<Image> defaultTextureImage;

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
     * @brief The model's joints
     */
    std::vector<Joint> joints;

    /**
     * @brief Default constructor
     *
     */
    Model();

    /**
     * @brief Constructor
     *
     * @param file     The file parsing object from which to load the model
     * @param meshName The name of the model mesh in the file
     *
     */
    Model(File& file, const std::string& meshName);

    /**
     * @brief Constructor (rvalue file - helps for declaring the file on the fly
     *                     when using gcc)
     *
     * @param file     The file parsing object from which to load the model
     * @param meshName The name of the model mesh in the file
     *
     */
    Model(File&& file, const std::string& meshName);

    /**
     * @brief Get the index of the current animation pose
     * @return The index of the current animation pose
     */

    uint32_t getCurrentPoseIdx();

    /**
     * @brief Advance joint animation pose (if joints are animated)
     */
    void animate();

    /**
     * @brief Get a joint transform, also calculating the transorms of the parent
     *        joints in the same tree and the animations, if any exist.
     *  @param jointIdx The index of the joint in the list of joints
     *  @return The transform
     */
    glm::mat4 getJointTransform(size_t jointIdx);

    friend class GlbFile;
    friend class Renderer;

  };
}
