/**
 * @file  Renderer.hpp
 * @brief Header of the Renderer class
 *
 *  Created on: 2014/10/19
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#pragma once

#if defined(__ANDROID__)
#include <android/asset_manager.h>
#elif defined(SMALL3D_IOS)
 // todo: Add ios includes if necessary
#else
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
    VkDescriptorSet orthoDescriptorSet = VK_NULL_HANDLE;
  };

  /**
   * @brief World uniform buffer object, containing the perspective matrix,
   *        light direction and camera transformation and offset. Used
   *        internally
   */
  struct UboWorld {
    glm::mat4 perspectiveMatrix;
    glm::vec3 lightDirection;
    float padding1;
    glm::mat4x4 cameraTransformation;
    glm::vec3 cameraOffset;
    float padding2[13];
  };

  /**
   * @brief Model placement uniform buffer object. Used internally
   */
  struct UboModelPlacement {
    glm::mat4x4 modelTransformation;
    glm::vec3 modelOffset;
    float padding[13];
  };

  /**
   * @brief Model colour uniform buffer object. Used internally
   */
  struct UboColour {
    glm::vec4 modelColour;
    float padding[12];
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

    static int realScreenWidth, realScreenHeight;

    std::string shadersPath = "";

    uint32_t maxObjectsPerPass;

    uint32_t perspectivePipelineIndex = 100;
    uint32_t orthographicPipelineIndex = 100;

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
    UboWorld* uboWorldDetailsDynamic = nullptr;
    size_t uboWorldDetailsDynamicSize = 0;

    std::vector<VkBuffer> lightIntensityBuffers;
    std::vector<VkDeviceMemory> lightIntensityBufferMemories;

    std::vector<VkBuffer> colourBuffersDynamic;
    std::vector<VkDeviceMemory> colourBuffersDynamicMemory;

    size_t dynamicColourAlignment = 0;
    UboColour* uboColourDynamic = nullptr;
    uint32_t colourMemIndex = 0;
    size_t uboColourDynamicSize = 0;

    VkSampler textureSampler;

    std::vector<VkImageView> boundTextureViews;

    std::allocator<char> alloc;

    float frustumScale = 0.0f;
    float zNear = 0.0f;
    float zFar = 0.0f;
    float zOffsetFromCamera = 0.0f;

    std::unordered_map<std::string, VulkanImage> textures;

    FT_Library library = 0;
    std::vector<float> textMemory;
    std::unordered_map<std::string, FT_Face> fontFaces;

#ifdef __ANDROID__
    std::vector<AAsset*> fontAssets;
#endif

    static std::vector<Model> nextModelsToDraw;

    static VkVertexInputBindingDescription bd[3];
    static VkVertexInputAttributeDescription ad[3];

    static VkDescriptorSetLayout descriptorSetLayout;
    static VkDescriptorSet descriptorSet;
    static VkDescriptorSetLayout textureDescriptorSetLayout;
    static VkDescriptorSetLayout perspectiveLayouts[2];

    static VkVertexInputBindingDescription orthobd[2];
    static VkVertexInputAttributeDescription orthoad[2];

    static VkDescriptorSetLayout orthoDescriptorSetLayout;
    static VkDescriptorSet orthoDescriptorSet;
    static VkDescriptorSetLayout textureOrthoDescriptorSetLayout;
    static VkDescriptorSetLayout orthographicLayouts[2];

    const uint32_t worldDescBinding = 0;
    const uint32_t modelPlacementDescBinding = 1;

    const uint32_t colourDescBinding = 2;
    const uint32_t lightDescBinding = 3;
    const uint32_t textureDescBinding = 4;

    const uint32_t textureDescBindingOrtho = 0;
    const uint32_t colourDescBindingOrtho = 1;

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

    static int setOrthoInputStateCallback(VkPipelineVertexInputStateCreateInfo*
      inputStateCreateInfo);
    static int setOrthoPipelineLayoutCallback(VkPipelineLayoutCreateInfo*
      pipelineLayoutCreateInfo);

    int bindOrthoBuffers(VkCommandBuffer commandBuffer,
      const Model& model);
    void recordOrthoDrawCommand(VkCommandBuffer commandBuffer,
      VkPipelineLayout pipelineLayout, const Model& model,
      uint32_t swapchainImageIndex);

    void initVulkan();

    VkDescriptorPool descriptorPool;
    bool descriptorPoolCreated = false;
    void createDescriptorPool();

    VkDescriptorPool orthoDescriptorPool;
    bool orthoDescriptorPoolCreated = false;
    void createOrthoDescriptorPool();

    VkCommandBuffer nextCommandBuffer;

    std::vector<Model> garbageModels;

    void allocateDescriptorSets();
    void updateDescriptorSets();

    void allocateOrthoDescriptorSets();
    void updateOrthoDescriptorSets();

    void setColourBuffer(glm::vec4 colour, uint32_t memIndex);

    void positionNextModel(const glm::vec3 offset,
      const glm::vec3 rotation,
      uint32_t memIndex);

    VulkanImage getTextureHandle(const std::string name) const;
    VulkanImage generateTexture(const std::string name,
      const float* data,
      const unsigned long width,
      const unsigned long height,
      const bool replace);

    void init(const int width, const int height,
      const std::string shadersPath);
    void initWindow(int& width, int& height);

    void setPerspectiveAndLight(bool perspective);

    // On Android and iOS, it is useful to be able to destroy and recreate the
    // renderer, so it is not provided only as a singleton for that platform.
    // By the way, do NOT create a renderer using getInstance and then
    // try to delete it in code. That will make an app crash. Either
    // create it with getInstance and assume it is a singleton which
    // will be destroyed automatically when the program terminates,
    // or instantiate it with "new" if you would like to delete it
    // later.
#if defined(__ANDROID__) || defined(SMALL3D_IOS)
  public:
    Renderer(const std::string& windowTitle = "",
      const int width = 0,
      const int height = 0,
      const float frustumScale = 1.0f,
      const float zNear = 1.0f,
      const float zFar = 24.0f,
      const float zOffsetFromCamera = -1.0f,
      const std::string& shadersPath =
#ifndef SMALL3D_IOS
      "resources/shaders/",
#else // On iOS "resources" is the name of a special folder, so it cannot be
      // used for small3d resources.
      "resources1/shaders/",
#endif
      const uint32_t maxObjectsPerPass = 20);
#else
    Renderer(const std::string& windowTitle, const int width,
      const int height, const float frustumScale, const float zNear,
      const float zFar, const float zOffsetFromCamera,
      const std::string& shadersPath, const uint32_t maxObjectsPerPass);
#endif

    Renderer();

#if !defined(__ANDROID__) && !defined(SMALL3D_IOS)
  public:
#endif

    /**
     * @brief Vector, indicating the direction of the light in the scene.
     */
    glm::vec3 lightDirection;

    /**
     * @brief The camera position in world space.
     */
    glm::vec3 cameraPosition;

    /**
     * @brief The camera rotation (around the x, y and z axes)
     */
    glm::vec3 cameraRotation;

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
     * @param windowTitle       The title of the game's window
     * @param width             The width of the window. If width and height are
     *                          not set or set to 0, the game will run in full
     *                          screen mode.
     * @param height            The height of the window
     * @param frustumScale	How much the frustum scales the items rendered
     * @param zNear		Projection plane z coordinate (use positive
     *                          value)
     * @param zFar		Far end of frustum z coordinate (use positive
     *                          value)
     * @param zOffsetFromCamera	The position of the projection plane with regard
     *                          to the camera.
     * @param shadersPath	The path where the shaders will be stored,
     *                          relative to the application's executing
     *                          directory. It defaults to the path provided by
     *                          the engine, but	it can be changed, so as to
     *                          accommodate for executables which are going to
     *                          be using it. Even though the path to the folder
     *                          can be changed, the folder structure within it
     *                          and the names of the shaders must remain as
     *                          provided. The shader code can be changed,
     *                          provided that their inputs and outputs are
     *                          maintained the same.
     * @param maxObjectsPerPass Maximum number of Models and / or SceneObjects
     *                          that will be rendered on the screen during a render
     *                          pass at the same time. This is used to pre-allocate
     *                          the needed memory buffers.
     * @return                  The Renderer object. It can only be assigned to
     *                          a pointer by its address (Renderer *r =
     *                          &Renderer::getInstance(...), since declaring
     *                          another Renderer variable and assigning to it
     *                          would invoke the default constructor, which has
     *                          been deleted.
     */
    static Renderer& getInstance(const std::string& windowTitle = "",
      const int width = 0,
      const int height = 0,
      const float frustumScale = 1.0f,
      const float zNear = 1.0f,
      const float zFar = 24.0f,
      const float zOffsetFromCamera = -1.0f,
      const std::string& shadersPath =
      "resources/shaders/",
      const uint32_t maxObjectsPerPass = 20);

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
      const std::string& fontPath =
#ifndef SMALL3D_IOS
      "resources/fonts/CrusoeText/CrusoeText-Regular.ttf",
#else
      "resources1/fonts/CrusoeText/CrusoeText-Regular.ttf",
#endif
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
     * @param offset      The offset (position) where to draw the model
     *                    (ignored in orthographic rendering)
     * @param rotation    Rotation (x, y, z)
     *                    (ignored in orthographic rendering)
     * @param colour      The colour of the model
     * @param textureName The name of the texture to attach to the model
     *                    (optional). The texture has to have been generated
     *                    already. If this is set, the colour parameter will
     *                    be ignored.
     * @param perspective If true perform perspective rendering, otherwise
     *                    orthographic.
     */
    void render(Model& model, const glm::vec3& offset, const glm::vec3& rotation,
      const glm::vec4& colour, const std::string& textureName = "",
      const bool perspective = true);

    /**
     * @brief Render a Model.
     * @param model       The model
     * @param offset      The offset (position) where to draw the model
     * @param rotation    Rotation (x, y, z)
     * @param textureName The name of the texture to attach to the model.
     *                    The texture has to have been generated already.
     */
    void render(Model& model, const glm::vec3& offset, const glm::vec3& rotation,
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
    void clearBuffers(Model& model) const;

    /**
     * @brief Clear an SceneObject (multiple models) from the GPU buffers
     *        (the SceneObject itself remains intact).
     * @param sceneObject The scene object
     */
    void clearBuffers(SceneObject& sceneObject) const;

    /**
     * @brief This function does not do anything in this edition (Vulkan) of
     *        small3d, since the screen is automatically cleared every time
     *        swapBuffers() is invoked. It still exists though so as not to
     *        break compatibility with code written for the OpenGL edition.
     */
    void clearScreen() const;

    /**
     * @brief In this edition (Vulkan) of small3d this function only sets the
     *        screen clearing colour. The  screen is automatically cleared every
     *        time swapBuffers() is invoked anyway. The name of the function has
     *        remained the same so as not to break compatibility with code
     *        written for the OpenGL edition.
     * @param colour The screen clearing colour
     */
    void clearScreen(const glm::vec4& colour);

    /**
     * @brief Swap the buffers.
     */
    void swapBuffers();

    Renderer(Renderer const&) = delete;
    void operator=(Renderer const&) = delete;
    Renderer(Renderer&&) = delete;
    void operator=(Renderer&&) = delete;

  };
}
