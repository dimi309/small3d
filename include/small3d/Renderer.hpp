/**
 * @file  Renderer.hpp
 * @brief Header of the Renderer class
 *
 *  Created on: 2014/10/19
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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
  struct vulkanImage {
    VkImageView imageView;
    VkImage image;
    VkDeviceMemory imageMemory;
    VkDescriptorSet descriptorSet;
    VkDescriptorSet orthoDescriptorSet;
  };

  struct UboOrientation {
    glm::mat4x4 xRotationMatrix;
    glm::mat4x4 yRotationMatrix;
    glm::mat4x4 zRotationMatrix;
    glm::vec3 offset;
    float padding[13];
  };

  struct UboColour {
    glm::vec4 colour;
    float padding[4];
  };

  /**
   * @class Renderer
   * @brief Renderer class
   */
  class Renderer
  {

  private:

    GLFWwindow* window;

    std::string windowTitle = "";

    int realScreenWidth = 0, realScreenHeight = 0;

    uint32_t perspectivePipelineIndex = 100;
    uint32_t orthographicPipelineIndex = 100;

    uint32_t currentSwapchainImageIndex = 0;

    std::vector<VkBuffer> renderOrientationBuffersDynamic;
    std::vector<VkDeviceMemory> renderOrientationBuffersDynamicMemory;

    std::vector<VkBuffer> cameraOrientationBuffers;
    std::vector<VkDeviceMemory> cameraOrientationBufferMemories;

    std::vector<VkBuffer> worldDetailsBuffers;
    std::vector<VkDeviceMemory> worldDetailsBufferMemories;
    
    std::vector<VkBuffer> lightIntensityBuffers;
    std::vector<VkDeviceMemory> lightIntensityBufferMemories;

    std::vector<VkBuffer> colourBuffersDynamic;
    std::vector<VkDeviceMemory> colourBuffersDynamicMemory;

    VkSampler textureSampler;
    
    std::vector<VkImageView> boundTextureViews;

    std::allocator<char> alloc;

    float frustumScale = 0.0f;
    float zNear = 0.0f;
    float zFar = 0.0f;
    float zOffsetFromCamera = 0.0f;

    std::unordered_map<std::string, vulkanImage> textures;

    FT_Library library = 0;
    std::vector<float> textMemory;
    std::unordered_map<std::string, FT_Face> fontFaces;

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

    size_t dynamicOrientationAlignment;
    UboOrientation* uboOrientationDynamic;
    uint32_t orientationMemIndex = 0;
    size_t uboOrientationDynamicSize;

    size_t dynamicColourAlignment;
    UboColour* uboColourDynamic;
    uint32_t colourMemIndex = 0;
    size_t uboColourDynamicSize;

    static int setInputStateCallback(VkPipelineVertexInputStateCreateInfo* inputStateCreateInfo);
    static int setPipelineLayout(VkPipelineLayoutCreateInfo* pipelineLayoutCreateInfo);
    static int bindBuffers(VkCommandBuffer commandBuffer, const Model& model);
    void recordDrawCommand(VkCommandBuffer commandBuffer,
      VkPipelineLayout pipelineLayout, const Model& model,
      uint32_t swapchainImageIndex);
    static int setOrthoInputState(VkPipelineVertexInputStateCreateInfo* inputStateCreateInfo);
    static int setOrthoPipelineLayout(VkPipelineLayoutCreateInfo* pipelineLayoutCreateInfo);
    static int bindOrthoBuffers(VkCommandBuffer commandBuffer, const Model& model);
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
    VkCommandBuffer nextOrthoCommandBuffer;

    std::vector<Model> garbageModels;

    void allocateDescriptorSets();
    void updateDescriptorSets();

    void allocateOrthoDescriptorSets();
    void updateOrthoDescriptorSets();

    void setColourBuffer(glm::vec4 colour, uint32_t memIndex);

    void positionNextObject(const glm::vec3 offset,
			    const glm::vec3 rotation, uint32_t memIndex);
    void positionCamera();
    vulkanImage getTextureHandle(const std::string name) const;
    vulkanImage generateTexture(const std::string name, const float *data,
			   const unsigned long width,
			   const unsigned long height);

    void init(const int width, const int height, 
      const std::string shadersPath);
    void initWindow(int &width, int &height);

    void setPerspectiveAndLight();

    Renderer(const std::string windowTitle, const int width, 
      const int height, const float frustumScale, const float zNear, 
      const float zFar, const float zOffsetFromCamera, 
      const std::string shadersPath);
    
    Renderer();
    
  public:
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
     * @return                  The Renderer object. It can only be assigned to 
     *                          a pointer by its address (Renderer *r =
     *                          &Renderer::getInstance(...), since declaring
     *                          another Renderer variable and assigning to it
     *                          would invoke the default constructor, which has
     *                          been deleted.
     */
    static Renderer& getInstance(const std::string windowTitle = "",
				 const int width = 0, 
				 const int height = 0,
				 const float frustumScale = 1.0f,
				 const float zNear = 1.0f,
				 const float zFar = 24.0f,
				 const float zOffsetFromCamera = -1.0f,
				 const std::string shadersPath =
				 "resources/shaders/");

    /**
     * @brief Destructor
     */
    ~Renderer();

    /**
     * @brief Get the GLFW window object, associated with the Renderer.
     */
    GLFWwindow* getWindow() const;

    /**
     * @brief Generate a texture on the GPU from the given image
     * @param name The name by which the texture will be known
     * @param image The image from which the texture will be generated
     */
    void generateTexture(const std::string name, const Image image);

    /**
     * @brief Generate a texture on the GPU that contains the given text
     * @param name     The name by which the texture will be known
     * @param text     The text that will be contained in the texture
     * @param colour   The colour of the text
     * @param fontSize The size of the font which will be used
     * @param fontPath Path to the TrueType font (.ttf) which will be used
     */
    void generateTexture(const std::string name, const std::string text,
			 const glm::vec3 colour,
			 const int fontSize,
			 const std::string fontPath =
			 "resources/fonts/CrusoeText/CrusoeText-Regular.ttf");

    /**
     * @brief Deletes the texture indicated by the given name.
     *
     * @param	name	The name of the texture.
     */
    void deleteTexture(const std::string name);

    /**
     * @brief Render a rectangle, using two of its corners that are diagonally
     *        opposed to each other to position it.
     * @param textureName The name of the texture to be used (must have been
     *                    generated with generateTexture())
     * @param topLeft     Where to place the top left corner
     * @param bottomRight Where to place the bottom right corner
     * @param perspective If set to true, use perspective rendering.
     *                    Otherwise use orthographic rendering.
     * @param colour      The colour of the rectangle (RGBA). If this is set,
     *                    textureName will be ignored.
     */
    void renderRectangle(const std::string textureName, const glm::vec3 topLeft,
			 const glm::vec3 bottomRight,
			 const bool perspective = false,
			 const glm::vec4 colour =
			 glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));

    /**
     * @brief Render a rectangle, using two of its corners that are diagonally
     *        opposed to each other to position it.
     * @param colour      The colour of the rectangle (RGBA)
     * @param topLeft     Where to place the top left corner 
     * @param bottomRight Where to place the bottom right corner 
     * @param perspective If set to true, use perspective rendering.
     *                    Otherwise use orthographic rendering.
     */
    void renderRectangle(const glm::vec4 colour, const glm::vec3 topLeft,
			 const glm::vec3 bottomRight, 
			 const bool perspective = false);
    
    /**
     * @brief Render a Model
     * @param model       The model
     * @param offset      The offset (position) where to draw the model
     * @param rotation    Rotation (x, y, z)
     * @param colour      The colour of the model
     * @param textureName The name of the texture to attach to the model
     *                    (optional). The texture has to have been generated
     *                    already. If this is set, the colour parameter will
     *                    be ignored.
     * @param perspective True = perspective drawing, otherwise orthographic
     */
    void render(Model &model, const glm::vec3 offset, const glm::vec3 rotation, 
		const glm::vec4 colour, const std::string textureName="", const bool perspective = true);

    /**
     * @brief Render a Model.
     * @param model       The model
     * @param offset      The offset (position) where to draw the model
     * @param rotation    Rotation (x, y, z)
     * @param textureName The name of the texture to attach to the model.
     *                    The texture has to have been generated already.
     */
    void render(Model &model, const glm::vec3 offset, const glm::vec3 rotation,
		const std::string textureName);

    /**
     * @brief Render a SceneObject
     * @param sceneObject The object
     * @param colour The colour the object. 
     */
    void render(SceneObject &sceneObject, const glm::vec4 colour);

    /**
     * @brief Render a SceneObject
     * @param sceneObject The object
     * @param textureName The name of the texture to attach to the object.
     *                    The texture has to have been generated already. 
     */
    void render(SceneObject &sceneObject, const std::string textureName);

    /**
     * @brief Render some text on the screen.
     * @param text The text to be rendered
     * @param colour      The colour in which the text will be rendered (r, g, b)
     * @param topLeft     Where to place the top left corner of the text
     *                    rectangle
     * @param bottomRight Where to place the bottom right corner of the text
     *                    rectangle
     * @param fontSize    The size of the font which will be used
     * @param fontPath    Path to the TrueType font (.ttf) which will be used
     */
    void write(const std::string text, const glm::vec3 colour,
	       const glm::vec2 topLeft, const glm::vec2 bottomRight,
	       const int fontSize=48,
	       const std::string fontPath =
	       "resources/fonts/CrusoeText/CrusoeText-Regular.ttf");

    /**
     * @brief Clear a Model from the GPU buffers (the object itself remains
     *        intact).
     * @param model The model
     */
    void clearBuffers(Model &model) const;

    /**
     * @brief Clear an object (multiple models) from the GPU buffers 
     * (the object itself remains intact).
     * @param model The model
     */
    void clearBuffers(SceneObject& sceneObject) const;

    /**
     * @brief Clears the screen.
     */
    void clearScreen() const;

    /**
     * @brief Clears the screen.
     * @param colour The colour with which the screen is to be cleared
     */
    void clearScreen(const glm::vec4 colour);

    /**
     * @brief Swap the buffers.
     */
    void swapBuffers();

    Renderer(Renderer const&) = delete;
    void operator=(Renderer const&) = delete;
    Renderer(Renderer &&) = delete;
    void operator=(Renderer &&) = delete;

  };
  
}
