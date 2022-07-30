 /*
 * Renderer.cpp
 *
 *  Created on: 2014/10/19
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#include "Renderer.hpp"
#include <stdexcept>
#include <fstream>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "BasePath.hpp"

namespace small3d {

  static void error_callback(int error, const char* description)
  {
    LOGERROR(std::string(description));
  }

  static std::string openglErrorToString(GLenum error);

  int Renderer::realScreenWidth;
  int Renderer::realScreenHeight;

  void Renderer::framebufferSizeCallback(GLFWwindow* window, int width,
    int height) {
    realScreenWidth = width;
    realScreenHeight = height;

    glViewport(0, 0, static_cast<GLsizei>(realScreenWidth),
      static_cast<GLsizei>(realScreenHeight));

    LOGDEBUG("New framebuffer dimensions " + std::to_string(width) + " x " +
      std::to_string(height));

  }

  std::string Renderer::loadShaderFromFile(const std::string& fileLocation)
    const {
    std::string shaderSource = "";
    std::string fullPath = getBasePath() + fileLocation;
    std::ifstream file(fullPath.c_str());
    std::string line;
    if (file.is_open()) {
      while (std::getline(file, line)) {
        shaderSource += line + "\n";
      }
    }
    return shaderSource;
  }

  GLuint Renderer::compileShader(const std::string& shaderSourceFile,
    const uint32_t shaderType) const {
    GLuint shader = glCreateShader(shaderType);
    std::string shaderSource = this->loadShaderFromFile(shaderSourceFile);
    if (shaderSource.length() == 0) {
      throw std::runtime_error("Shader source file '" + shaderSourceFile +
        "' is empty or not found.");
    }
    const char* shaderSourceChars = shaderSource.c_str();
    glShaderSource(shader, 1, &shaderSourceChars, NULL);
    glCompileShader(shader);

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if (status == GL_FALSE) {
      throw std::runtime_error(
        "Failed to compile shader:\n" + shaderSource +
        "\n" + this->getShaderInfoLog(shader));
    }
    else {
      LOGDEBUG("Shader " + shaderSourceFile + " compiled successfully.");
    }
    return shader;
  }

  std::string Renderer::getProgramInfoLog(const GLuint linkedProgram) const {

    GLint infoLogLength;
    glGetProgramiv(linkedProgram, GL_INFO_LOG_LENGTH, &infoLogLength);

    GLchar* infoLog = new GLchar[static_cast<size_t>(infoLogLength) + 1];
    GLsizei lengthReturned = 0;
    glGetProgramInfoLog(linkedProgram, infoLogLength, &lengthReturned, infoLog);

    std::string infoLogStr(infoLog);

    if (lengthReturned == 0) {
      infoLogStr = "(No info)";
    }
    delete[] infoLog;
    return infoLogStr;
  }

  std::string Renderer::getShaderInfoLog(const GLuint shader) const {

    GLint infoLogLength;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

    GLchar* infoLog = new GLchar[static_cast<size_t>(infoLogLength) + 1];
    GLsizei lengthReturned = 0;
    glGetShaderInfoLog(shader, infoLogLength, &lengthReturned, infoLog);

    std::string infoLogStr(infoLog);
    if (lengthReturned == 0) {
      infoLogStr = "(No info)";
    }

    delete[] infoLog;

    return infoLogStr;
  }

  void Renderer::initOpenGL() {

    glewExperimental = GL_TRUE;

    GLenum initResult = glewInit();

    if (initResult != GLEW_OK) {
      throw std::runtime_error("Error initialising GLEW");
    }
    else {
      std::string glewVersion = reinterpret_cast<char*>
        (const_cast<GLubyte*>(glewGetString(GLEW_VERSION)));
      LOGDEBUG("Using GLEW version " + glewVersion);
    }

    checkForOpenGLErrors("initialising GLEW", false);

    LOGINFO("OpenGL version: " +
      std::string(reinterpret_cast<char*>
      (const_cast<GLubyte*>(glGetString(GL_VERSION)))));

#ifdef __APPLE__
    GLboolean clientStorage = 0;
    glGetBooleanv(GL_APPLE_client_storage, &clientStorage);
    if (clientStorage == GL_TRUE) {
      LOGDEBUG("GL_APPLE_client_storage is activated.");
    }
    else {
      LOGDEBUG("GL_APPLE_client_storage is not activated.");
    }
#endif

  }

  void Renderer::checkForOpenGLErrors(const std::string& when, const bool abort)
    const {
    GLenum errorCode = glGetError();
    if (errorCode != GL_NO_ERROR) {
      LOGERROR("OpenGL error while " + when);

      do {
        LOGERROR(openglErrorToString(errorCode));
        errorCode = glGetError();
      } while (errorCode != GL_NO_ERROR);

      if (abort)
        throw std::runtime_error("OpenGL error while " + when);
    }
  }

  void Renderer::transform(Model& model, const glm::vec3& offset,
    const glm::mat4x4& rotation) const {
    
    GLint modelTransformationUniformLocation = glGetUniformLocation(shaderProgram,
      "modelTransformation");

    glm::mat4x4 modelTranformation = 
      rotation *
      glm::scale(glm::mat4x4(1.0f), model.scale) *
      glm::translate(glm::mat4x4(1.0f), model.origTranslation) *
      glm::toMat4(model.origRotation) *
      glm::scale(glm::mat4x4(1.0f), model.origScale) * model.origTransformation;
    
    glUniformMatrix4fv(modelTransformationUniformLocation, 1, GL_FALSE,
      glm::value_ptr(modelTranformation));

    uint32_t hasJoints = model.joints.size() > 0 ? 1U : 0U;
    GLint hasJointsUniform = glGetUniformLocation(shaderProgram, "hasJoints");
    glUniform1ui(hasJointsUniform, hasJoints);

    glm::mat4 jointTransformations[Model::MAX_JOINTS_SUPPORTED];

    uint64_t idx = 0;
    for (auto& joint : model.joints) {

      jointTransformations[idx] =

        glm::inverse(glm::translate(glm::mat4x4(1.0f), model.origTranslation) *
          glm::toMat4(model.origRotation) *
          glm::scale(glm::mat4x4(1.0f), model.origScale)) *

        model.getJointTransform(idx) *

        joint.inverseBindMatrix;

      ++idx;
    }

    auto jointTransformationsUniformLocation = glGetUniformLocation(shaderProgram, "jointTransformations");
    glUniformMatrix4fv(jointTransformationsUniformLocation, 16, GL_FALSE, glm::value_ptr(jointTransformations[0]));

    GLint offsetUniform = glGetUniformLocation(shaderProgram, "modelOffset");
    glUniform3fv(offsetUniform, 1, glm::value_ptr(offset));
  }

  GLuint Renderer::getTextureHandle(const std::string& name) const {
    GLuint handle = 0;
    auto nameTexturePair = textures.find(name);
    if (nameTexturePair != textures.end()) {
      handle = nameTexturePair->second;
    }
    return handle;
  }

  GLuint Renderer::generateTexture(const std::string& name, const float* data,
    const unsigned long width,
    const unsigned long height,
    const bool replace) {

    bool found = false;

    GLuint textureHandle;

    for (auto& nameTexturePair : textures) {
      if (nameTexturePair.first == name) {
        if (!replace) {
          throw std::runtime_error("Texture with name " + name +
            " already exists and replace flag not set.");
        }
        found = true;
        break;
      }
    }

    if (found) {
      deleteTexture(name);
    }

    glGenTextures(1, &textureHandle);
    glBindTexture(GL_TEXTURE_2D, textureHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA,
      GL_FLOAT, data);

    textures.insert(make_pair(name, textureHandle));
    glBindTexture(GL_TEXTURE_2D, 0);

    return textureHandle;
  }

  void Renderer::init(const int width, const int height,
    const std::string& windowTitle,
    const std::string& shadersPath) {

    realScreenWidth = width;
    realScreenHeight = height;

    this->initWindow(realScreenWidth, realScreenHeight, windowTitle);

    this->initOpenGL();

    glViewport(0, 0, static_cast<GLsizei>(realScreenWidth),
      static_cast<GLsizei>(realScreenHeight));

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);

    glDepthRange(0.0f, 1.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLuint vertexShader = compileShader(shadersPath +
      "perspectiveMatrixLightedShader.vert",
      GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(shadersPath + "textureShader.frag",
      GL_FRAGMENT_SHADER);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    glLinkProgram(shaderProgram);

    GLint status;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
      throw std::runtime_error("Failed to link program:\n" +
        this->getProgramInfoLog(shaderProgram));
    }
    else {
      LOGDEBUG("Linked main rendering program successfully");
    }
    glDetachShader(shaderProgram, vertexShader);
    glDetachShader(shaderProgram, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);

    glUseProgram(0);

    Image blankImage("");
    blankImage.toColour(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
    generateTexture("blank", blankImage);
  }

  void Renderer::initWindow(int& width, int& height,
    const std::string& windowTitle) {

    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
      throw std::runtime_error("Unable to initialise GLFW");
    }
#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // This was used as a workaround for an issue I cannot remember
    // on Mojave but on Monterey it was making the window transparent
    // when the colour of a rendered mesh was transparent so I have
    // commented it out.
    // glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#endif
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    GLFWmonitor* monitor = nullptr; // If NOT null, a full-screen window will
    // be created.

    bool fullScreen = false;

    if ((width == 0 && height != 0) || (width != 0 && height == 0)) {
      throw std::runtime_error("Screen width and height both have to be equal "
        "or not equal to zero at the same time.");
    }
    else if (width == 0) {

      monitor = glfwGetPrimaryMonitor();

      const GLFWvidmode* mode = glfwGetVideoMode(monitor);
      width = mode->width;
      height = mode->height;

      LOGINFO("Detected screen width " + std::to_string(width) + " and height " +
        std::to_string(height));

      fullScreen = true;
    }

    window = glfwCreateWindow(width, height, windowTitle.c_str(), monitor,
      nullptr);

    if (!window) {
      throw std::runtime_error("Unable to create GLFW window");
    }

    if (fullScreen) {
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    }

    glfwMakeContextCurrent(window);

    width = 0;
    height = 0;
    glfwGetFramebufferSize(window, &width, &height);

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    LOGINFO("Framebuffer width " + std::to_string(width) + " height " +
      std::to_string(height));

  }

  void Renderer::setWorldDetails(bool perspective) {

    GLint perspectiveMatrixUniform =
      glGetUniformLocation(shaderProgram, "perspectiveMatrix");

    glm::mat4x4 perspectiveMatrix = perspective && realScreenHeight != 0 ?
      glm::perspective(fieldOfView, static_cast<float>(realScreenWidth / realScreenHeight), zNear, zFar) :
      glm::mat4x4(1);

    glUniformMatrix4fv(perspectiveMatrixUniform, 1, GL_FALSE,
      glm::value_ptr(perspectiveMatrix));

    GLint lightDirectionUniform = glGetUniformLocation(shaderProgram,
      "lightDirection");
    glm::vec3 lightDirectionOut = perspective ?
      lightDirection : glm::vec3(0.0f, 0.0f, 0.0f);
    glUniform3fv(lightDirectionUniform, 1,
      glm::value_ptr(lightDirectionOut));

    GLint lightIntensityUniform = glGetUniformLocation(shaderProgram,
      "lightIntensity");
    glUniform1f(lightIntensityUniform, lightIntensity);

    GLint cameraTransformationUniform = glGetUniformLocation(shaderProgram,
      "cameraTransformation");

    glm::mat4x4 cameraTransformation = perspective ?
      this->cameraTransformation :
      glm::mat4x4(1);

    glUniformMatrix4fv(cameraTransformationUniform, 1, GL_FALSE,
      glm::value_ptr(cameraTransformation));

    GLint cameraOffsetUniform = glGetUniformLocation(shaderProgram,
      "cameraOffset");

    glm::vec3 cameraPositionOut = perspective ?
      cameraPosition : glm::vec3(0.0f, 0.0f, 0.0f);
    glUniform3fv(cameraOffsetUniform, 1, glm::value_ptr(cameraPositionOut));

  }

  void Renderer::bindTexture(const std::string& name) {
    GLuint textureHandle = getTextureHandle(name);

    if (textureHandle == 0) {
      throw std::runtime_error("Texture " + name +
        " has not been generated");
    }

    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, textureHandle);
    GLint loc = glGetUniformLocation(shaderProgram, "textureImage");

    glUniform1i(loc, 0);

  }

    void Renderer::clearScreen() const {
#ifdef __APPLE__
    // Needed to avoid transparent rendering in Mojave by default
    // (caused by the transparency hint in initWindow, which is
    // a workaround for a GLFW problem on that platform)
    // This used to be 0,0,0,1. Hopefully it still works.
    glClearColor(clearColour.x, clearColour.y, clearColour.z, clearColour.w);
#endif
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }


  Renderer::Renderer() {
    window = 0;
    shaderProgram = 0;
    noShaders = false;
  }

  Renderer::Renderer(const std::string& windowTitle, const int width,
    const int height, const float fieldOfView,
    const float zNear, const float zFar,
    const std::string& shadersPath,
    const uint32_t objectsPerFrame,
    const uint32_t objectsPerFrameInc) {

    window = 0;
    shaderProgram = 0;

    noShaders = false;

    this->zNear = zNear;
    this->zFar = zFar;
    this->fieldOfView = fieldOfView;

    init(width, height, windowTitle, shadersPath);

    FT_Error ftError = FT_Init_FreeType(&library);

    if (ftError != 0) {
      throw std::runtime_error("Unable to initialise font system");
    }

    // Generate VAO
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
  }

  void Renderer::setCameraRotation(const glm::vec3& rotation) {
    cameraRotationByMatrix = false;
    this->cameraRotationXYZ = rotation;
    this->cameraTransformation = glm::rotate(glm::mat4x4(1.0f), -rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)) *
      glm::rotate(glm::mat4x4(1.0f), -rotation.x, glm::vec3(1.0f, 0.0f, 0.0f)) *
      glm::rotate(glm::mat4x4(1.0f), -rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
  }

  void Renderer::rotateCamera(const glm::vec3& rotation) {
    if (cameraRotationByMatrix) {
      throw std::runtime_error("Attempted x, y, z representation camera rotation, while having set the initial rotation by matrix.");
    }
    else {
      this->cameraRotationXYZ += rotation;
      this->cameraTransformation = glm::rotate(glm::mat4x4(1.0f), -this->cameraRotationXYZ.z, glm::vec3(0.0f, 0.0f, 1.0f)) *
	glm::rotate(glm::mat4x4(1.0f), -this->cameraRotationXYZ.x, glm::vec3(1.0f, 0.0f, 0.0f)) *
	glm::rotate(glm::mat4x4(1.0f), -this->cameraRotationXYZ.y, glm::vec3(0.0f, 1.0f, 0.0f));
    }
  }

  void Renderer::setCameraTransformation(const glm::mat4x4& rotation) {
    this->cameraTransformation = glm::inverse(rotation);
    cameraRotationByMatrix = true;
    cameraRotationXYZ = glm::vec3(0.0f);
  }

  const glm::vec3 Renderer::getCameraOrientation() const {
    auto orientationVec4 = glm::inverse(this->cameraTransformation) * glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);
    return glm::vec3(orientationVec4.x, orientationVec4.y, orientationVec4.z);
  }

  const glm::mat4x4 Renderer::getCameraRotation() const {
    return glm::inverse(this->cameraTransformation);
  }

  const glm::vec3 Renderer::getCameraRotationXYZ() const {
    if (cameraRotationByMatrix) {
      throw std::runtime_error("Attempted x, y, z representation camera rotation retrieval, while having set the rotation by matrix.");
    }
    return this->cameraRotationXYZ;
  }

  Renderer& Renderer::getInstance(const std::string& windowTitle,
    const int width, const int height,
    const float fieldOfView,
    const float zNear, const float zFar,
    const std::string& shadersPath,
    const uint32_t objectsPerFrame,
    const uint32_t objectsPerFrameInc) {

    initLogger();

    static Renderer instance(windowTitle, width, height, fieldOfView, zNear,
      zFar, shadersPath, objectsPerFrame, objectsPerFrameInc);
    return instance;
  }

  Renderer::~Renderer() {
    LOGDEBUG("Renderer destructor running");
    glBindTexture(GL_TEXTURE_2D, 0);

    //At times, this has caused crashes on MacOS
    for (auto it = textures.begin();
      it != textures.end(); ++it) {
      LOGDEBUG("Deleting texture " + it->first);
      glDeleteTextures(1, &it->second);
    }
    
    for (auto idFacePair : fontFaces) {
      FT_Done_Face(idFacePair.second);
    }

    FT_Done_FreeType(library);

    if (!noShaders) {
      glUseProgram(0);

      glDeleteVertexArrays(1, &vao);
      glBindVertexArray(0);

    }

    if (shaderProgram != 0) {
      glDeleteProgram(shaderProgram);
    }
      //This was causing crashes on MacOS
      //glfwTerminate();
  }

  GLFWwindow* Renderer::getWindow() const {
    return window;
  }

  void Renderer::generateTexture(const std::string& name, const Image& image) {
    LOGDEBUG("Sending image to GPU, dimensions " + std::to_string(image.getWidth()) +
	     ", " + std::to_string(image.getHeight()));
    this->generateTexture(name, image.getData(), image.getWidth(),
      image.getHeight(), true);
  }

  void Renderer::generateTexture(const std::string& name, const std::string& text,
    const glm::vec3& colour, const int fontSize,
    const std::string& fontPath,
    const bool replace) {

    std::string faceId = std::to_string(fontSize) + fontPath;

    auto idFacePair = fontFaces.find(faceId);
    FT_Face face;
    FT_Error error;

    if (idFacePair == fontFaces.end()) {
      std::string faceFullPath = getBasePath() + fontPath;
      LOGDEBUG("Loading font from " + faceFullPath);
      error = FT_New_Face(library, faceFullPath.c_str(), 0, &face);
      if (error != 0) {
        throw std::runtime_error("Failed to load font from " + faceFullPath);
      }
      else {
        LOGDEBUG("Font loaded successfully");
        fontFaces.insert(make_pair(faceId, face));
      }
    }
    else {
      face = idFacePair->second;
    }

    // Multiplying by 64 to convert to 26.6 fractional points. Using 100dpi.
    error = FT_Set_Char_Size(face, 64 * fontSize, 0, 100, 0);

    if (error != 0) {
      throw std::runtime_error("Failed to set font size.");
    }

    size_t width = 0, maxTop = 0, height = 0;

    // Figure out bitmap dimensions
    for (const char& c : text) {
      error = FT_Load_Char(face, (FT_ULong)c, FT_LOAD_RENDER);
      if (error != 0) {
        throw std::runtime_error("Failed to load character glyph.");
      }
      FT_GlyphSlot slot = face->glyph;
      width += slot->advance.x / 64;
      if (maxTop < static_cast<unsigned long>(slot->bitmap_top))
        maxTop = static_cast<unsigned long>(slot->bitmap_top);
    }

    height = maxTop + static_cast<unsigned long>(0.3 * maxTop);

    textMemory.resize(4 * width * height * sizeof(float));
    memset(&textMemory[0], 0, 4 * width * height * sizeof(float));

    unsigned long totalAdvance = 0;

    for (const char& c : text) {
      error = FT_Load_Char(face, (FT_ULong)c, FT_LOAD_RENDER);
      if (error != 0) {
        throw std::runtime_error("Failed to load character glyph.");
      }

      FT_GlyphSlot slot = face->glyph;

      if (slot->bitmap.width * slot->bitmap.rows > 0) {
        for (int row = 0; row < static_cast<int>(slot->bitmap.rows); ++row) {
          for (int col = 0; col < static_cast<int>(slot->bitmap.width); ++col) {

            glm::vec4 colourAlpha = glm::vec4(colour, 0.0f);

            colourAlpha.a =
              floorf(100.0f * (static_cast<float>
              (slot->bitmap.buffer[row * slot->bitmap.width +
                col]) /
                255.0f) + 0.5f) / 100.0f;

            memcpy(&textMemory[4 * (maxTop -
              slot->bitmap_top
              + row)* static_cast<size_t>(width) +
              4 *
              (static_cast<size_t>(slot->bitmap_left) +
                static_cast<size_t>(col))
              + totalAdvance],
              glm::value_ptr(colourAlpha),
              4 * sizeof(float));
          }
        }
      }
      totalAdvance += 4 * static_cast<unsigned long>(slot->advance.x / 64);
    }
    generateTexture(name, &textMemory[0], static_cast<unsigned long>(width), static_cast<unsigned long>(height), replace);
  }

  void Renderer::deleteTexture(const std::string& name) {
    auto nameTexturePair = textures.find(name);

    if (nameTexturePair != textures.end()) {
      glBindTexture(GL_TEXTURE_2D, 0);
      glDeleteTextures(1, &(nameTexturePair->second));
      textures.erase(name);
    }
  }

  void Renderer::createRectangle(Model& rect,
    const glm::vec3& topLeft,
    const glm::vec3& bottomRight) {

    rect.vertexData = {
      bottomRight.x, bottomRight.y, bottomRight.z, 1.0f,
      bottomRight.x, topLeft.y, topLeft.z, 1.0f,
      topLeft.x, topLeft.y, topLeft.z, 1.0f,
      topLeft.x, bottomRight.y, bottomRight.z, 1.0f
    };

    rect.vertexDataByteSize = 16 * sizeof(float);

    rect.indexData = {
      0, 1, 2,
      2, 3, 0
    };

    rect.indexDataByteSize = 6 * sizeof(uint32_t);

    rect.normalsData = std::vector<float>(12);
    rect.normalsDataByteSize = 12 * sizeof(float);

    rect.textureCoordsData = {
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,
        0.0f, 1.0f
    };
    rect.textureCoordsDataByteSize = 8 * sizeof(float);
  }

  void Renderer::render(Model& model, const glm::vec3& position, const glm::vec3& rotation,
    const glm::vec4& colour, const std::string& textureName,
    const bool perspective) {

    this->render(model, position, glm::rotate(glm::mat4x4(1.0f), rotation.y, glm::vec3(0.0f, 1.0f, 0.0f))*
      glm::rotate(glm::mat4x4(1.0f), rotation.x, glm::vec3(1.0f, 0.0f, 0.0f))*
      glm::rotate(glm::mat4x4(1.0f), rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)),
      colour, textureName, perspective);

  }

  void Renderer::render(Model& model, const glm::vec3& position, const glm::vec3& rotation,
    const std::string& textureName) {

    this->render(model, position, glm::rotate(glm::mat4x4(1.0f), rotation.y, glm::vec3(0.0f, 1.0f, 0.0f)) *
      glm::rotate(glm::mat4x4(1.0f), rotation.x, glm::vec3(1.0f, 0.0f, 0.0f)) *
      glm::rotate(glm::mat4x4(1.0f), rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)),
      textureName);

  }

  void Renderer::render(Model& model, const glm::vec3& offset,
    const glm::mat4x4& rotation,
    const glm::vec4& colour,
    const std::string& textureName,
    const bool perspective) {

    if (!perspective) {
      glClearDepth(1.0f);
    }

    unsigned const attrib_position = 0;
    unsigned const attrib_normal = 1;
    unsigned const attrib_joint = 2;
    unsigned const attrib_weight = 3;
    unsigned const attrib_uv = 4;

    glUseProgram(shaderProgram);

    bool alreadyInGPU = model.positionBufferObjectId != 0;

    if (!alreadyInGPU) {
      glGenBuffers(1, &model.indexBufferObjectId);
      glGenBuffers(1, &model.positionBufferObjectId);
      glGenBuffers(1, &model.normalsBufferObjectId);
      glGenBuffers(1, &model.uvBufferObjectId);
      glGenBuffers(1, &model.jointBufferObjectId);
      glGenBuffers(1, &model.weightBufferObjectId);
    }

    // Vertices
    glBindBuffer(GL_ARRAY_BUFFER, model.positionBufferObjectId);
    if (!alreadyInGPU) {
      glBufferData(GL_ARRAY_BUFFER,
        model.vertexDataByteSize,
        model.vertexData.data(),
        GL_STATIC_DRAW);
    }

    // Vertex indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.indexBufferObjectId);
    if (!alreadyInGPU) {
      glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        model.indexDataByteSize,
        model.indexData.data(),
        GL_STATIC_DRAW);
    }

    glEnableVertexAttribArray(attrib_position);
    glVertexAttribPointer(attrib_position, 4, GL_FLOAT, GL_FALSE, 0, 0);

    // Normals
    
    glBindBuffer(GL_ARRAY_BUFFER, model.normalsBufferObjectId);
    
      
    if (!alreadyInGPU) {
      if (model.normalsDataByteSize > 0) {
	glBufferData(GL_ARRAY_BUFFER,
		     model.normalsDataByteSize,
		     model.normalsData.data(),
		     GL_STATIC_DRAW);
      } else {
	// The normals buffer is created with 0 values if the corresponding
	// data does not exist, otherwise there can be a EXC_BAD_ACCESS
	// when running glDrawElements on MacOS.
	size_t ns = (model.vertexDataByteSize / 4) * 3;
	std::unique_ptr<char[]> data = std::make_unique<char[]>(ns);
	glBufferData(GL_ARRAY_BUFFER,
		     ns,
		     &data[0],
		     GL_STATIC_DRAW);
      }
    }
      
    glEnableVertexAttribArray(attrib_normal);
    glVertexAttribPointer(attrib_normal, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    
    if (model.jointDataByteSize != 0) {
      glBindBuffer(GL_ARRAY_BUFFER, model.jointBufferObjectId);
      if (!alreadyInGPU) {
        glBufferData(GL_ARRAY_BUFFER,
          model.jointDataByteSize,
          model.jointData.data(),
          GL_STATIC_DRAW);
      }
        
        glEnableVertexAttribArray(attrib_joint);
        glVertexAttribIPointer(attrib_joint, 4, GL_UNSIGNED_BYTE, 0, 0);
    }

    if (model.weightDataByteSize != 0) {
      glBindBuffer(GL_ARRAY_BUFFER, model.weightBufferObjectId);
      if (!alreadyInGPU) {
        glBufferData(GL_ARRAY_BUFFER,
          model.weightDataByteSize,
          model.weightData.data(),
          GL_STATIC_DRAW);
      }

      glEnableVertexAttribArray(attrib_weight);
      glVertexAttribPointer(attrib_weight, 4, GL_FLOAT, GL_FALSE, 0, 0);

    }

    // Find the colour uniform
    GLint colourUniform = glGetUniformLocation(shaderProgram, "modelColour");

    if (textureName != "") {

      // "Disable" colour since there is a texture
      glUniform4fv(colourUniform, 1,
        glm::value_ptr(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)));

      bindTexture(textureName);

      // UV Coordinates

      glBindBuffer(GL_ARRAY_BUFFER, model.uvBufferObjectId);

      if (!alreadyInGPU) {
        glBufferData(GL_ARRAY_BUFFER,
          model.textureCoordsDataByteSize,
          model.textureCoordsData.data(),
          GL_STATIC_DRAW);
      }
      
      glEnableVertexAttribArray(attrib_uv);
      glVertexAttribPointer(attrib_uv, 2, GL_FLOAT, GL_FALSE, 0, 0);
    }
    else {
      // If there is no texture, use the given colour
      glUniform4fv(colourUniform, 1, glm::value_ptr(colour));
      bindTexture("blank");
    }

    setWorldDetails(perspective);

    transform(model, offset, rotation);

    // Draw
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDrawElements(GL_TRIANGLES,
      static_cast<GLsizei>(model.indexData.size()),
      GL_UNSIGNED_INT, 0);

    glDisableVertexAttribArray(attrib_position);
    glDisableVertexAttribArray(attrib_normal);
    if (model.jointDataByteSize != 0) glDisableVertexAttribArray(attrib_joint);
    if (model.weightDataByteSize != 0) glDisableVertexAttribArray(attrib_weight);
    if (textureName != "") glDisableVertexAttribArray(attrib_uv);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    glUseProgram(0);
  }

  void Renderer::render(Model& model, const glm::vec3& offset,
    const glm::mat4x4& rotation,
    const std::string& textureName) {
    this->render(model, offset, rotation, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
      textureName);
  }

  void Renderer::render(Model& model, const std::string& textureName,
    const bool perspective) {
    this->render(model, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
      textureName, perspective);
  }

  void Renderer::render(Model& model, const glm::vec4& colour,
    const bool perspective) {
    this->render(model, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), colour,
      "", perspective);
  }

  void Renderer::render(SceneObject& sceneObject,
    const glm::vec4& colour) {
    this->render(sceneObject.getModel(), sceneObject.position,
      sceneObject.transformation, colour, "");
  }

  void Renderer::render(SceneObject& sceneObject,
    const std::string& textureName) {
    this->render(sceneObject.getModel(), sceneObject.position,
      sceneObject.transformation, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
      textureName);
  }

  void Renderer::clearBuffers(Model& model) const {
    if (model.positionBufferObjectId != 0) {
      glDeleteBuffers(1, &model.positionBufferObjectId);
      model.positionBufferObjectId = 0;
    }

    if (model.indexBufferObjectId != 0) {
      glDeleteBuffers(1, &model.indexBufferObjectId);
      model.indexBufferObjectId = 0;
    }
    if (model.normalsBufferObjectId != 0) {
      glDeleteBuffers(1, &model.normalsBufferObjectId);
      model.normalsBufferObjectId = 0;
    }

    if (model.uvBufferObjectId != 0) {
      glDeleteBuffers(1, &model.uvBufferObjectId);
      model.uvBufferObjectId = 0;
    }
  }

  void Renderer::clearBuffers(SceneObject& sceneObject) const {
    for (auto model : *sceneObject.models) {
      clearBuffers(model);
    }
  }

  void Renderer::setBackgroundColour(const glm::vec4& colour) {
    clearColour = colour;
    glClearColor(clearColour.x, clearColour.y, clearColour.z, clearColour.w);
  }

  void Renderer::swapBuffers() const {
    glfwSwapBuffers(window);
    clearScreen();
  }

  /**
   * Convert error enum returned from OpenGL to a readable string error message.
   * @param error The error code returned from OpenGL
   */
  std::string openglErrorToString(GLenum error) {

    std::string errorString;

    switch (error) {
    case GL_NO_ERROR:
      errorString = "GL_NO_ERROR: No error has been recorded. "
        "The value of this symbolic constant is guaranteed to be 0.";
      break;
    case GL_INVALID_ENUM:
      errorString = "GL_INVALID_ENUM: An unacceptable value is specified for "
        "an enumerated argument. The offending command is ignored and has no "
        "other side effect than to set the error flag.";
      break;
    case GL_INVALID_VALUE:
      errorString = "GL_INVALID_VALUE: A numeric argument is out of range. "
        "The offending command is ignored and has no other side effect than "
        "to set the error flag.";
      break;
    case GL_INVALID_OPERATION:
      errorString = "GL_INVALID_OPERATION: The specified operation is not "
        "allowed in the current state. The offending command is ignored "
        "and has no other side effect than to set the error flag.";
      break;
    case GL_INVALID_FRAMEBUFFER_OPERATION:
      errorString = "GL_INVALID_FRAMEBUFFER_OPERATION: The framebuffer "
        "object is not complete. The offending command is ignored and has "
        "no other side effect than to set the error flag.";
      break;
    case GL_OUT_OF_MEMORY:
      errorString = "GL_OUT_OF_MEMORY: There is not enough memory left to "
        "execute the command. The state of the GL is undefined, except for "
        "the state of the error flags, after this error is recorded.";
      break;
    case GL_STACK_UNDERFLOW:
      errorString = "GL_STACK_UNDERFLOW: An attempt has been made to perform "
        "an operation that would cause an internal stack to underflow.";
      break;
    case GL_STACK_OVERFLOW:
      errorString = "GL_STACK_OVERFLOW: An attempt has been made to perform "
        "an operation that would cause an internal stack to overflow.";
      break;
    default:
      errorString = "Unknown error";
      break;
    }
    return errorString;
  }

}

