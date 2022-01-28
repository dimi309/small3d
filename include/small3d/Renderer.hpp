/**
 * @file  Renderer.hpp
 * @brief The small3d renderer
 *
 *  Created on: 2014/10/19
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#pragma once

#if defined(__ANDROID__)
#include <android/asset_manager.h>
#elif !defined(SMALL3D_IOS)
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#endif

#include "Logger.hpp"
#include "Image.hpp"
#include "Model.hpp"
#include "SceneObject.hpp"

#include <unordered_map>
#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

#ifndef SMALL3D_IOS
#define DEFAULT_SHADERS_DIR "resources/shaders/"
#define DEFAULT_FONT_PATH "resources/fonts/CrusoeText/CrusoeText-Regular.ttf"
#else // On iOS "resources" is the name of a special folder, so it cannot be
      // used for small3d resources.
#define DEFAULT_SHADERS_DIR "resources1/shaders/"
#define DEFAULT_FONT_PATH "resources1/fonts/CrusoeText/CrusoeText-Regular.ttf"
#endif

namespace small3d
{
  /**
   * @brief Structure used to keep track of information related to images
   *        created on the GPU. Used internally
   */
  struct VulkanImage {
    VkImageView imageView = VK_NULL_HANDLE;
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory imageMemory = VK_NULL_HANDLE;
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    std::shared_ptr<std::vector<float>> data; // Using a pointer here to avoid
                                              // copying data when manipulating this
                                              // structure.
    unsigned long width = 0; 
    unsigned long height = 0;

    VulkanImage() {
      data = std::make_shared<std::vector<float>>();
    }
  };

  /**
   * @brief World uniform buffer object, containing the perspective matrix,
   *        light direction and camera transformation and offset. Used
   *        internally
   */
  struct UboWorldDetails {
    glm::mat4 perspectiveMatrix;
    glm::vec3 lightDirection;
    float padding1;
    glm::mat4x4 cameraTransformation;
    glm::vec3 cameraOffset;
    float padding2[25]; // Paddings seem to work when the number of floats (not bytes)
                        // add up to powers of two for each ubo.
  };

  /**
   * @brief Model placement uniform buffer object. Used internally
   */
  struct UboModelPlacement {
    glm::mat4x4 modelTransformation;
    glm::mat4x4 jointTransformations[Model::MAX_JOINTS_SUPPORTED];
    glm::vec3 modelOffset;
    uint32_t hasJoints = 0U;
    float padding[236];
  };

  /**
   * @brief Model colour uniform buffer object. Used internally
   */
  struct UboColour {
    glm::vec4 modelColour;
    float padding[60];
  };

  /**
   * @class Renderer
   * @brief The renderer (Vulkan)
   */
  class Renderer
  {

  private:

#if !defined(__ANDROID__) && !defined(SMALL3D_IOS)
    GLFWwindow* window;
#endif

    std::string windowTitle = "";
    glm::vec4 backgroundColour = glm::vec4(0.0f);

    static int realScreenWidth, realScreenHeight;

    std::string shadersPath = "";

    static const uint32_t defaultObjectsPerFrame = 200;
    static const uint32_t defaultObjectsPerFrameInc = 1000;

    uint32_t objectsPerFrame = 0;
    uint32_t objectsPerFrameInc = 0;
   
    uint32_t pipelineIndex = 100;

    uint32_t currentSwapchainImageIndex = 0;

    std::vector<VkBuffer> renderModelPlacementBuffersDynamic;
    std::vector<VkDeviceMemory> renderModelPlacementBuffersDynamicMemory;

    size_t dynamicModelPlacementAlignment = 0;
    UboModelPlacement* uboModelPlacementDynamic = nullptr;
    uint32_t modelPlacementMemIndex = 0;
    size_t uboModelPlacementDynamicSize = 0;

    std::vector<VkBuffer> worldDetailsBuffersDynamic;
    std::vector<VkDeviceMemory> worldDetailsBuffersDynamicMemory;

    size_t dynamicWorldDetailsAlignment = 0;
    UboWorldDetails* uboWorldDetailsDynamic = nullptr;
    size_t uboWorldDetailsDynamicSize = 0;

    std::vector<VkBuffer> lightIntensityBuffers;
    std::vector<VkDeviceMemory> lightIntensityBufferMemories;

    std::vector<VkBuffer> colourBuffersDynamic;
    std::vector<VkDeviceMemory> colourBuffersDynamicMemory;

    size_t dynamicColourAlignment = 0;
    UboColour* uboColourDynamic = nullptr;
    uint32_t colourMemIndex = 0;
    size_t uboColourDynamicSize = 0;

    VkSampler textureSampler = VK_NULL_HANDLE;

    std::allocator<char> alloc;

    float fieldOfView = 0.0f;
    float zNear = 0.0f;
    float zFar = 0.0f;

    std::unordered_map<std::string, VulkanImage> textures;
    std::unordered_map<VkBuffer, VkDeviceMemory> allocatedBufferMemory;

    FT_Library library = 0;
    std::vector<float> textMemory;
    std::unordered_map<std::string, FT_Face> fontFaces;

#ifdef __ANDROID__
    std::vector<AAsset*> fontAssets;
#endif

    static std::vector<Model> nextModelsToDraw;

    static VkVertexInputBindingDescription bd[5];
    static VkVertexInputAttributeDescription ad[5];

    static VkDescriptorSetLayout descriptorSetLayout;
    static VkDescriptorSet descriptorSet;
    static VkDescriptorSetLayout textureDescriptorSetLayout;
    static VkDescriptorSetLayout perspectiveLayouts[2];

    const uint32_t worldDescBinding = 0;
    const uint32_t modelPlacementDescBinding = 1;

    const uint32_t colourDescBinding = 2;
    const uint32_t lightDescBinding = 3;
    const uint32_t textureDescBinding = 4;

#if !defined(__ANDROID__) && !defined(SMALL3D_IOS)
    static void framebufferSizeCallback(GLFWwindow* window, int width,
      int height);
#endif

    static int setInputStateCallback(VkPipelineVertexInputStateCreateInfo*
      inputStateCreateInfo);
    static int setPipelineLayoutCallback(VkPipelineLayoutCreateInfo*
      pipelineLayoutCreateInfo);

    int bindBuffers(VkCommandBuffer commandBuffer, const Model& model);
    void recordDrawCommand(VkCommandBuffer commandBuffer,
      VkPipelineLayout pipelineLayout, const Model& model,
      uint32_t swapchainImageIndex, bool perspective);

    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    void createDescriptorPool();
    void destroyDescriptorPool();

    VkCommandBuffer nextCommandBuffer = VK_NULL_HANDLE;

    std::vector<Model> garbageModels;

    void allocateDescriptorSets();
    void updateDescriptorSets();
    void destroyDescriptorSets();

    void setColourBuffer(glm::vec4 colour, uint32_t memIndex);

    void transform(Model& model, const glm::vec3 offset,
      const glm::mat4x4 rotation,
      uint32_t memIndex);

    VulkanImage getTextureHandle(const std::string name) const;
    void generateTexture(const std::string name,
      const float* data,
      const unsigned long width,
      const unsigned long height,
      const bool replace);

    void init(const int width, const int height,
      const std::string shadersPath);
    
    void increaseObjectsPerFrame(const uint32_t additionalObjects);
    
    void allocateDynamicBuffers();
    void destroyDynamicBuffers();
    void initWindow(int& width, int& height);

    void setWorldDetails(bool perspective);
    void setLightIntensity();

    void deleteImageFromGPU(VulkanImage &gpuImage);

    glm::mat4x4 cameraRotation = glm::mat4x4(1);
    glm::vec3 cameraRotationXYZ = glm::vec3(0);
    bool cameraRotationByMatrix = false;

    uint32_t nextModelRenderIndex = 1;
    uint32_t memoryResetModelRenderIndex = 0;

    Renderer(const std::string& windowTitle, const int width,
      const int height, const float fieldOfView, const float zNear,
      const float zFar, 
      const std::string& shadersPath, const uint32_t objectsPerFrame,
      const uint32_t objectsPerFrameInc);

    Renderer();

  public:

    /**
     * @brief Vector, indicating the direction of the light in the scene.
     *        It points towards a directional light source.
     */
    glm::vec3 lightDirection = glm::vec3(0.0f, 0.4f, 0.5f);

    /**
     * @brief The camera position in world space. Ignored for orthographic
     *        rendering.
     */
    glm::vec3 cameraPosition = glm::vec3(0, 0, 0);

    /**
     * @brief: Set the rotation of the camera
     *
     * @param rotation The rotation (x, y, z)
     */
    void setCameraRotation(const glm::vec3& rotation);

    /**
     * @brief: Modify the rotation of the camera
     *
     * @param rotation The rotation to modify by (x, y, z)
     */
    void rotateCamera(const glm::vec3& rotation);

    /**
     * @brief: Set the rotation of the camera
     *         by transformation matrix
     *
     * @param rotation The rotation tranformation matrix
     */
    void setCameraRotation(const glm::mat4x4& rotation);

    /**
     * @brief: Get the orientation of the camera
     *
     * @return The orientation of the camera
     */
    const glm::vec3 getCameraOrientation() const;

    /**
     * @brief: Get the rotation of the camera
     *         by transformation matrix
     *
     * @return The camera tranformation matrix (this is inversed
     *         when rendering)
     */
    const glm::mat4x4 getCameraRotation() const;

    /**
     * @brief: Get the rotation of the camera in x, y, z representation.
     *         This will NOT work if the rotation was set via the
     *         setRotation(mat4x4) function.
     *
     * @return The rotation in x, y, z representation (this is negated
     *         when rendering)
     */
    const glm::vec3 getCameraRotationXYZ() const;

    /**
     * @brief Get the real screen width
     * @return The screen width
     */
    int getScreenWidth();

    /**
     * @brief Get the real screen height
     * @return The screen height
     */
    int getScreenHeight();

    /**
     * @brief The light intensity (set to -1.0f if no lighting is to be used).
     */
    float lightIntensity = 1.0f;

    /**
     * @brief Get the instance of the Renderer (the Renderer is a singleton).
     * @param windowTitle        The title of the game's window
     * @param width              The width of the window. If width and height are
     *                           not set or set to 0, the game will run in full
     *                           screen mode.
     * @param height             The height of the window
     * @param fieldOfView        Field of view in radians (angle between the top and the bottom plane
     *                           of the view frustum).
     * @param zNear              Projection plane z coordinate (use positive
     *                           value)
     * @param zFar               Far end of frustum z coordinate (use positive
     *                           value)
     * @param shadersPath        The path where the shaders will be stored,
     *                           relative to the application's executing
     *                           directory. It defaults to the path provided by
     *                           the engine, but	it can be changed, so as to
     *                           accommodate for executables which are going to
     *                           be using it. Even though the path to the folder
     *                           can be changed, the folder structure within it
     *                           and the names of the shaders must remain as
     *                           provided. The shader code can be changed,
     *                           provided that their inputs and outputs are
     *                           maintained the same.
     * @param objectsPerFrame    Maximum number of Models and / or SceneObjects
     *                           foreseen to be rendered per frame. This is used 
     *                           to pre-allocate the needed memory buffers.
     * @param objectsPerFrameInc Number of objects to increase number of objects per
     *                           frame by, each time the latter is exceeded.
     * @return                   The Renderer object. It can only be assigned to
     *                           a pointer by its address (Renderer *r =
     *                           &Renderer::getInstance(...), since declaring
     *                           another Renderer variable and assigning to it
     *                           would invoke the default constructor, which has
     *                           been deleted.
     */
    static Renderer& getInstance(const std::string& windowTitle = "",
      const int width = 0,
      const int height = 0,
      const float fieldOfView = 0.785f, 
      const float zNear = 1.0f,
      const float zFar = 24.0f,
      const std::string& shadersPath =
      DEFAULT_SHADERS_DIR,
      const uint32_t objectsPerFrame = defaultObjectsPerFrame,
      const uint32_t objectsPerFrameInc = defaultObjectsPerFrameInc);

    /**
     * @brief Destructor
     */
    ~Renderer();

#if !defined(__ANDROID__) && !defined(SMALL3D_IOS)
    /**
     * @brief Get the GLFW window object, associated with the Renderer.
     */
    GLFWwindow* getWindow() const;
#endif

    /**
     * @brief Generate a texture on the GPU from the given image
     * @param name The name by which the texture will be known
     * @param image The image from which the texture will be generated
     */
    void generateTexture(const std::string& name, const Image& image);

    /**
     * @brief Generate a texture on the GPU that contains the given text
     * @param name     The name by which the texture will be known
     * @param text     The text that will be contained in the texture
     * @param colour   The colour of the text
     * @param fontSize The size of the font which will be used
     * @param fontPath Path to the TrueType font (.ttf) which will be used
     * @param replace  If true, an exception will be thrown if a texture
     *                 with the same name already exists. Otherwise it will
     *                 be overwritten.
     */
    void generateTexture(const std::string& name, const std::string& text,
      const glm::vec3& colour,
      const int fontSize = 48,
      const std::string& fontPath = DEFAULT_FONT_PATH,
      const bool replace = true);

    /**
     * @brief Deletes the texture indicated by the given name.
     *
     * @param	name	The name of the texture.
     */
    void deleteTexture(const std::string& name);

    /**
    * @brief Populates a Model object with a rectangle stretching between the
    *        given coordinates.
    * @param rect        Model object in which the rectangle data will be entered
    * @param topLeft     Top left corner of the rectangle
    * @param bottomRight Bottom right corner of the rectangle
    */
    void createRectangle(Model& rect,
      const glm::vec3& topLeft,
      const glm::vec3& bottomRight);

    /**
     * @brief Render a Model
     * @param model       The model
     * @param position    The position of the model (x, y, z)
     * @param rotation    Rotation (x, y, z)
     * @param colour      The colour of the model
     * @param textureName The name of the texture to attach to the model
     *                    (optional). The texture has to have been generated
     *                    already. If this is set, the colour parameter will
     *                    be ignored.
     * @param perspective If true perform perspective rendering, otherwise
     *                    orthographic.
     */
    void render(Model& model, const glm::vec3& position, const glm::vec3& rotation,
      const glm::vec4& colour, const std::string& textureName = "",
      const bool perspective = true);

    /**
     * @brief Render a Model.
     * @param model       The model
     * @param position    The position of the model (x, y, z)
     * @param rotation    Rotation (x, y, z)
     * @param textureName The name of the texture to attach to the model.
     *                    The texture has to have been generated already.
     */
    void render(Model& model, const glm::vec3& position, const glm::vec3& rotation,
      const std::string& textureName);

    /**
     * @brief Render a Model
     * @param model       The model
     * @param position    The position of the model (x, y, z)
     * @param rotation    Rotation transformation matrix 
     * @param colour      The colour of the model
     * @param textureName The name of the texture to attach to the model
     *                    (optional). The texture has to have been generated
     *                    already. If this is set, the colour parameter will
     *                    be ignored.
     * @param perspective If true perform perspective rendering, otherwise
     *                    orthographic.
     */
    void render(Model& model, const glm::vec3& position, const glm::mat4x4& rotation,
      const glm::vec4& colour, const std::string& textureName = "",
      const bool perspective = true);

    /**
     * @brief Render a Model.
     * @param model       The model
     * @param position    The position of the model (x, y, z)
     * @param rotation    Rotation transformation matrix 
     * @param textureName The name of the texture to attach to the model.
     *                    The texture has to have been generated already.
     */
    void render(Model& model, const glm::vec3& position, const glm::mat4x4& rotation,
      const std::string& textureName);

    /**
     * @brief Render a Model.
     * @param model       The model
     * @param textureName The name of the texture to attach to the model.
     *                    The texture has to have been generated already.
     * @param perspective True = perspective drawing, otherwise orthographic
     */
    void render(Model& model, const std::string& textureName,
      const bool perspective = true);

    /**
     * @brief Render a Model
     * @param model       The model
     * @param colour      The colour of the model
     * @param perspective True = perspective drawing, otherwise orthographic
     */
    void render(Model& model, const glm::vec4& colour,
      const bool perspective = true);


    /**
     * @brief Render a SceneObject
     * @param sceneObject The object
     * @param colour The colour the object.
     */
    void render(SceneObject& sceneObject, const glm::vec4& colour);

    /**
     * @brief Render a SceneObject
     * @param sceneObject The object
     * @param textureName The name of the texture to attach to the object.
     *                    The texture has to have been generated already.
     */
    void render(SceneObject& sceneObject, const std::string& textureName);

    /**
     * @brief Clear a Model from the GPU buffers (the object itself remains
     *        intact).
     * @param model The model
     */
    void clearBuffers(Model& model);

    /**
     * @brief Clear an SceneObject (multiple models) from the GPU buffers
     *        (the SceneObject itself remains intact).
     * @param sceneObject The scene object
     */
    void clearBuffers(SceneObject& sceneObject);

    /**
     * @brief Set the background colour of the screen.
     * @param colour The background colour
     */
    void setBackgroundColour(const glm::vec4& colour);

    /**
     * @brief Swap the buffers.
     */
    void swapBuffers();

    /**
     * @brief Set up the Vulkan. This is called by the constructor and used internally. 
     *        Normally there is no need to invoke it, appart from when
     *        an application runs on a mobile platform, in which case it can be useful
     *        to call destroyVulkan when the app loses focus and then 
     *        setupVulkan when it regains it.
     */
    void setupVulkan();

    /**
     * @brief Destroy Vulkan. This is is called by the destructor and used internally. 
     *        Normally there is no need to invoke it, appart
     *        from when an application runs on a mobile platform, in which case it 
     *        can be useful to call destroyVulkan when the app loses
     *        focus and then setupVulkan when it regains it.
     */
    void destroyVulkan();

    Renderer(Renderer const&) = delete;
    void operator=(Renderer const&) = delete;
    Renderer(Renderer&&) = delete;
    void operator=(Renderer&&) = delete;

  };
}
