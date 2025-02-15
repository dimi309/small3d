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
#include "Math.hpp"
#include "Image.hpp"
#include "File.hpp"
#include "Material.hpp"

namespace glm {
  template<class Archive> void serialize(Archive& archive, small3d::Vec3& v) { archive(v.x, v.y, v.z); }
  template<class Archive> void serialize(Archive& archive, small3d::Vec4& v) { archive(v.x, v.y, v.z, v.w); }
  template<class Archive> void serialize(Archive& archive, small3d::Mat4& m) { archive(m[0], m[1], m[2], m[3]); }
}

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

    uint32_t positionBufferObjectId = 0;
    uint32_t indexBufferObjectId = 0;
    uint32_t normalsBufferObjectId = 0;
    uint32_t uvBufferObjectId = 0;
    uint32_t jointBufferObjectId = 0;
    uint32_t weightBufferObjectId = 0;

    uint32_t currentAnimation = 0;
    std::vector<uint64_t> numPoses;

    // Original transformation matrix (from armature/skin),
    // as read from a file
    Mat4 origTransformation = Mat4(1.0f);

    // Original rotation (from armature/skin), as read from a
    // file (in quaternion form)
    Quat origRotation = { 0.0f, 0.0f, 0.0f, 1.0f };

    // brief Original translation, as read from a file
    Vec3 origTranslation = Vec3(0.0f, 0.0f, 0.0f);

    // brief Original scale, as read from a file
    Vec3 origScale = Vec3(1.0f, 1.0f, 1.0f);


  public:

    /**
     *  @brief The material of the model
     */
    Material material;

    /**
     * @brief animation component
     */
    struct AnimationComponent {
      uint32_t input = 0;
      std::vector<Quat> rotationAnimation;
      std::vector<Vec3> translationAnimation;
      std::vector<Vec3> scaleAnimation;
      std::vector<float> times;
      template <class Archive>
      void serialize(Archive& archive) {
        archive(input, rotationAnimation, translationAnimation, scaleAnimation, times);
      }
    };

    /**
     *  @brief animation
     */
    struct Animation {
      std::string name;
      std::vector<AnimationComponent> animationComponents;
      template <class Archive>
      void serialize(Archive& archive) {
        archive(name, animationComponents);
      }
    };

    /**
     * @brief joint
     */
    struct Joint {
      uint32_t node = 0;
      std::string name;
      Mat4 inverseBindMatrix = Mat4(1.0f);
      Quat rotation = { 1.0f, 0.0f, 0.0f, 0.0f };
      Vec3 scale = Vec3(1.0f, 1.0f, 1.0f);
      Vec3 translation = Vec3(0.0f, 0.0f, 0.0f);
      Mat4 transformation = Mat4(1.0f);
      std::vector<uint32_t> children;
      std::vector<Animation> animations;

      template <class Archive>
      void serialize(Archive& archive) {
        archive(node, name, inverseBindMatrix, rotation, scale, translation,
          transformation, children, animations);
      }
    };

    /**
     * @brief Use this to scale the model and not origScale
     */
    Vec3 scale = Vec3(1.0f, 1.0f, 1.0f);

    /**
     * @brief Maximum number of supported joints
     *
     */
    static const uint32_t MAX_JOINTS_SUPPORTED = 32;

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
    std::vector<uint16_t> indexData;

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
     * @brief Animations of the model (there are also joint animations)
     */
    std::vector<Animation> animations;

    /**
     * @brief Should the model produce a shadow?
     */
    bool noShadow = false;

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
    explicit Model(File& file, const std::string& meshName = "") ;

    /**
     * @brief Constructor (rvalue file - helps for declaring the file on the fly
     *                     when using gcc)
     *
     * @param file     The file parsing object from which to load the model
     * @param meshName The name of the model mesh in the file
     *
     */
    explicit Model(File&& file, const std::string& meshName = "");

    /**
     * @brief Get the number of animation poses in the current animation
     * @return The number of animation poses
     */

    uint64_t getNumPoses();
    
    /**
     * @brief Get the number of available animations
     * @return The number of animations
     */
    size_t getNumAnimations();

    /**
     * @brief Set the current animation
     * @param animationIdx The index of the current animation
     */
    void setAnimation(uint32_t animationIdx);


    /**
     * @brief Get a transform of the model.
     *  @param animationIdx The index of the animation to use
     *  @param currentPose The pose of the animation to calculate the
     *         joint transformation for.
     *  @param seconds The animation moment (in seconds) - mostly used internally
     *  @return The transform
     */
    Mat4 getTransform(uint32_t animationIdx, uint64_t currentPose, float seconds = 0.0f);

    /**
     * @brief Get a joint transform, also calculating the transorms of the parent
     *        joints in the same tree and the animations, if any exist.
     *  @param jointIdx The index of the joint in the list of joints
     *  @param animationIdx The index of the animation to use
     *  @param currentPose The pose of the animation to calculate the
     *         joint transformation for.
     *  @param seconds The animation moment (in seconds) - mostly used internally
     *  @return The transform
     */
    Mat4 getJointTransform(size_t jointIdx, uint32_t animationIdx, uint64_t currentPose, float seconds = 0.0f);

    /**
     * @brief Get the Model's original scale (usually the one read from the file
     *        the Model was loaded from.
     */
    Vec3 getOriginalScale();

    /**
     * @brief Save model data in binary format
     * @param binaryFilePath Path of file to save binary data to.
     */
    void saveBinary(const std::string& binaryFilePath);

    template <class Archive>
    void serialize(Archive& archive) {
      archive(currentAnimation,
        numPoses,
        origTransformation,
        origRotation,
        origTranslation,
        origScale,
        material,
        scale,
        defaultTextureImage,
        vertexData,
        vertexDataByteSize,
        indexData,
        indexDataByteSize,
        normalsData,
        normalsDataByteSize,
        textureCoordsData,
        textureCoordsDataByteSize,
        jointData,
        jointDataByteSize,
        weightData,
        weightDataByteSize,
        joints,
        animations
        );
    }

    friend class GlbFile;
    friend class Renderer;
   
  };
}
