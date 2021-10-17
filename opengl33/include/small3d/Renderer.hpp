/**
 * @file  Renderer.hpp
 * @brief Header of the Renderer class
 *
 *  Created on: 2014/10/19
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#pragma once

#define GLEW_NO_GLU
#include <GL/glew.h>
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

  /**
   * @class Renderer
   * @brief Renderer class (OpenGL 3.3)
   */
  class Renderer
  {

  private:

    GLFWwindow* window;

    static int realScreenWidth, realScreenHeight;

    uint32_t shaderProgram = 0;
   
    uint32_t vao = 0;

    uint32_t renderOrientation = 0;
    uint32_t cameraOrientation = 0;
    uint32_t worldDetails = 0;
    uint32_t lightUboId = 0;
    uint32_t perspColourUboId = 0;
    uint32_t orthoColourUboId = 0;

    bool noShaders;

    float fieldOfView = 0.0f;
    float zNear = 0.0f;
    float zFar = 0.0f;

    glm::vec4 clearColour = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    
    std::unordered_map<std::string, uint32_t> textures;

    FT_Library library = 0;
    std::vector<float> textMemory;
    std::unordered_map<std::string, FT_Face> fontFaces;

    glm::mat4x4 cameraRotation = glm::mat4x4(1.0f);
    glm::vec3 cameraRotationXYZ = glm::vec3(0.0f);
    bool cameraRotationByMatrix = false;

    static void framebufferSizeCallback(GLFWwindow* window, int width,
					int height);

    std::string loadShaderFromFile(const std::string& fileLocation) const;
    uint32_t compileShader(const std::string& shaderSourceFile,
      const uint32_t shaderType) const;
    std::string getProgramInfoLog(const uint32_t linkedProgram) const;
    std::string getShaderInfoLog(const uint32_t shader) const;
    void initOpenGL();
    void checkForOpenGLErrors(const std::string& when, const bool abort) const;

    void transform(Model& model, const glm::vec3& offset,
      const glm::mat4x4& rotation) const;

    uint32_t getTextureHandle(const std::string& name) const;
    uint32_t generateTexture(const std::string& name, const float* data,
      const unsigned long width,
      const unsigned long height,
      const bool replace);

    void init(const int width, const int height, const std::string& windowTitle,
      const std::string& shadersPath);
    void initWindow(int& width, int& height,
      const std::string& windowTitle = "");

    void setWorldDetails(bool perspective);

    void bindTexture(const std::string& name);

    void clearScreen() const;

    Renderer(const std::string& windowTitle, const int width, const int height,
      const float fieldOfView, const float zNear, const float zFar,
      const std::string& shadersPath,
      const uint32_t maxObjectsPerPass);

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
    glm::vec3 cameraPosition  = glm::vec3(0, 0, 0);

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
    const glm::vec3 Renderer::getCameraOrientation() const;

    /**
     * @brief: Get the rotation of the camera
     *         by transformation matrix
     *
     * @return The camera tranformation matrix (this is inversed
     *         when rendering)
     */
    const glm::mat4x4 Renderer::getCameraRotation() const;

    /**
     * @brief: Get the rotation of the camera in axis-angle representation.
     *         This will NOT work if the rotation was set via the
     *         setRotation(mat4x4) function.
     *
     * @return The rotation in axis-angle representation (this is negated
     *         when rendering)
     */
    const glm::vec3 Renderer::getCameraRotationXYZ() const;

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
     * @param fieldOfView       Field of view in radians (angle between the top and the bottom plane
     * @param zNear             Projection plane z coordinate (use positive
     *                          value)
     * @param zFar              Far end of frustum z coordinate (use positive
     *                          value)
     * @param shadersPath       The path where the shaders will be stored,
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
     * @param maxObjectsPerPass Ignored parameter (used for compatibility with
     *                          Vulkan edition).
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
      const float fieldOfView = 0.785,
      const float zNear = 1.0f,
      const float zFar = 24.0f,
      const std::string& shadersPath =
      "resources/shaders/",
      const uint32_t maxObjectsPerPass = 20);

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
      "resources/fonts/CrusoeText/CrusoeText-Regular.ttf",
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
     * @brief Clear a Model from the GPU buffers (the Model itself remains
     *        intact).
     * @param model The model
     */
    void clearBuffers(Model& model) const;

    /**
     * @brief Clear a SceneObject (multiple models) from the GPU buffers
     *        (the SceneObject itself remains intact).
     * @param sceneObject The scene object
     */
    void clearBuffers(SceneObject& sceneObject) const;

    /**
     * @brief Set the background colour of the screen.
     * @param colour The background colour
     */
    void setBackgroundColour(const glm::vec4& colour);

    /**
     * @brief This is a double buffered system and this command swaps
     * the buffers.
     */
    void swapBuffers() const;

    Renderer(Renderer const&) = delete;
    void operator=(Renderer const&) = delete;
    Renderer(Renderer&&) = delete;
    void operator=(Renderer&&) = delete;

  };

}
