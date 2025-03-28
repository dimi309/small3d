/*
* Renderer.cpp
*
*  Created on: 2014/10/19
*      Author: Dimitri Kourkoulis
*     License: BSD 3-Clause License (see LICENSE file)
*/

#include "Renderer.hpp"

#ifdef _WIN32
#include <Windows.h>
#endif

#include <stdexcept>
#include <fstream>
#include "BasePath.hpp"

unsigned const attrib_position = 0;
unsigned const attrib_normal = 1;
unsigned const attrib_joint = 2;
unsigned const attrib_weight = 3;
unsigned const attrib_uv = 4;

namespace small3d {

  static std::string openglErrorToString(GLenum error);

  std::string Renderer::loadShaderFromFile(const std::string& fileLocation)
    const {
    std::string shaderSource = "";
    std::string fullPath = getBasePath() + fileLocation;
    

    std::ifstream file(fullPath.c_str());
    if (file.is_open()) {
      std::string line;
      while (std::getline(file, line)) {

        shaderSource += line + "\n";

      }
    }

    file.close();

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

  }

  void Renderer::checkForOpenGLErrors(const std::string& when, const bool abort)
    const {
    GLenum errorCode = glGetError();
    if (errorCode != GL_NO_ERROR) {
      if (abort) {
        LOGERROR("OpenGL error while " + when);
      }
      else {
        LOGDEBUG("OpenGL error while " + when);
      }

      do {
        if (abort) {
          LOGERROR(openglErrorToString(errorCode));
        }
        else {
          LOGDEBUG(openglErrorToString(errorCode));
        }
        errorCode = glGetError();
      } while (errorCode != GL_NO_ERROR);

      if (abort)
        throw std::runtime_error("OpenGL error while " + when);
    }
  }

  void Renderer::transform(Model& model, Vec3& offset,
    const Mat4& rotation, uint64_t currentPose) const {

    GLint modelTransformationUniformLocation = glGetUniformLocation(shaderProgram,
      "modelTransformation");

    Mat4 modelTransformation =
      rotation *
      scale(Mat4(1.0f), model.scale) *
      translate(Mat4(1.0f), model.origTranslation) *
      model.origRotation.toMatrix() *
      scale(Mat4(1.0f), model.origScale) * model.origTransformation *
      model.getTransform(model.currentAnimation, currentPose);

    glUniformMatrix4fv(modelTransformationUniformLocation, 1, GL_FALSE,
      Value_ptr(modelTransformation));

    int32_t hasJoints = model.joints.size() > 0 ? 1 : 0;
    GLint hasJointsUniform = glGetUniformLocation(shaderProgram, "hasJoints");
    glUniform1i(hasJointsUniform, hasJoints);

    Mat4 jointTransformations[Model::MAX_JOINTS_SUPPORTED];

    uint64_t idx = 0;
    for (const auto& joint : model.joints) {

      jointTransformations[idx] =
        model.getJointTransform(idx, model.currentAnimation, currentPose) *
        joint.inverseBindMatrix;
      ++idx;
    }
    auto jointTransformationsUniformLocation = glGetUniformLocation(shaderProgram, "jointTransformations");
    glUniformMatrix4fv(jointTransformationsUniformLocation, Model::MAX_JOINTS_SUPPORTED, GL_FALSE, Value_ptr(jointTransformations[0]));

    GLint offsetUniform = glGetUniformLocation(shaderProgram, "modelOffset");
    glUniform3fv(offsetUniform, 1, Value_ptr(offset));
  }

  GLuint Renderer::getTextureHandle(const std::string& name) const {
    GLuint handle = 0;
    auto nameTexturePair = textures.find(name);
    if (nameTexturePair != textures.end()) {
      handle = nameTexturePair->second;
    }
    return handle;
  }

  GLuint Renderer::generateTexture(const std::string& name, const uint8_t* data,
    const unsigned long width,
    const unsigned long height,
    const bool replace) {

    bool found = false;

    GLuint textureHandle;

    for (const auto& nameTexturePair : textures) {
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
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureHandle);


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
      GL_UNSIGNED_BYTE, data);

    textures.insert(make_pair(name, textureHandle));

    glBindTexture(GL_TEXTURE_2D, 0);

    return textureHandle;
  }

  void Renderer::init(const int width, const int height,
    const std::string& windowTitle,
    const std::string& shadersPath) {

    int tmpWidth = width;
    int tmpHeight = height;

    windowing.initWindow(tmpWidth, tmpHeight, windowTitle);

    this->initOpenGL();

    glViewport(0, 0, static_cast<GLsizei>(tmpWidth),
      static_cast<GLsizei>(tmpHeight));

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


    GLint colourTextureLocation = glGetUniformLocation(shaderProgram, "textureImage");
    GLint depthMapTextureLocation = glGetUniformLocation(shaderProgram, "shadowMap");

    glProgramUniform1i(shaderProgram, colourTextureLocation, 0);
    glProgramUniform1i(shaderProgram, depthMapTextureLocation, 1);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);

    glUseProgram(0);

    Image blankImage("");
    blankImage.toColour(Vec4(0.0f, 0.0f, 0.0f, 0.0f));
    generateTexture("blank", blankImage);
    LOGDEBUG("Blank image generated");


    glGenTextures(1, &depthMapTexture);

    glActiveTexture(GL_TEXTURE0 + 1);

    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
      depthMapTextureWidth, depthMapTextureHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);


    glGenFramebuffers(1, &depthMapFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, origFramebuffer);

  }

  
  void Renderer::setWorldDetails(bool perspective) {

    auto orthographicMatrix = ortho(-shadowSpaceSize, shadowSpaceSize, -shadowSpaceSize, shadowSpaceSize, -shadowSpaceSize, shadowSpaceSize);


    GLint perspectiveMatrixUniform =
      glGetUniformLocation(shaderProgram, "perspectiveMatrix");

    Mat4 perspectiveMatrix = perspective && windowing.realWindowHeight != 0 ?
      small3d::perspective(fieldOfView, static_cast<float>(windowing.realWindowWidth / windowing.realWindowHeight), zNear, zFar) :
      renderingDepthMap ? orthographicMatrix : Mat4(1.0f);

    glUniformMatrix4fv(perspectiveMatrixUniform, 1, GL_FALSE,
      Value_ptr(perspectiveMatrix));

    GLint lightDirectionUniform = glGetUniformLocation(shaderProgram,
      "lightDirection");
    Vec3 lightDirectionOut = perspective ?
      lightDirection : Vec3(0.0f, 0.0f, 0.0f);
    glUniform3fv(lightDirectionUniform, 1,
      Value_ptr(lightDirectionOut));

    GLint lightIntensityUniform = glGetUniformLocation(shaderProgram,
      "lightIntensity");

    glUniform1f(lightIntensityUniform, lightIntensity);

    GLint cameraTransformationUniform = glGetUniformLocation(shaderProgram,
      "cameraTransformation");

    Mat4 usedCameraTransformation = perspective || renderingDepthMap ?
      this->cameraTransformation :
      Mat4(1);

    glUniformMatrix4fv(cameraTransformationUniform, 1, GL_FALSE,
      Value_ptr(usedCameraTransformation));

    GLint cameraOffsetUniform = glGetUniformLocation(shaderProgram,
      "cameraOffset");

    Vec3 cameraPositionOut = perspective || renderingDepthMap ?
      cameraPosition : Vec3(0.0f, 0.0f, 0.0f);
    glUniform3fv(cameraOffsetUniform, 1, Value_ptr(cameraPositionOut));

    GLint lightSpaceMatrixUniform = glGetUniformLocation(shaderProgram,
      "lightSpaceMatrix");

    glUniformMatrix4fv(lightSpaceMatrixUniform, 1, GL_FALSE,
      Value_ptr(lightSpaceMatrix));

    GLint orthographicMatrixUniform = glGetUniformLocation(shaderProgram,
      "orthographicMatrix");
    glUniformMatrix4fv(orthographicMatrixUniform, 1, GL_FALSE,
      Value_ptr(orthographicMatrix));

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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  Renderer::Renderer() {

    shaderProgram = 0;
    noShaders = false;
  }

  Renderer::Renderer(const std::string& windowTitle, const int width,
    const int height, const float fieldOfView,
    const float zNear, const float zFar,
    const std::string& shadersPath,
    const uint32_t objectsPerFrame,
    const uint32_t objectsPerFrameInc) {

    start(windowTitle, width,
      height, fieldOfView,
      zNear, zFar,
      shadersPath,
      objectsPerFrame,
      objectsPerFrameInc);

    LOGDEBUG("Renderer constructor done.");
  }

  void Renderer::start(const std::string& windowTitle, const int width,
    const int height, const float fieldOfView,
    const float zNear, const float zFar,
    const std::string& shadersPath,
    const uint32_t objectsPerFrame,
    const uint32_t objectsPerFrameInc) {

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

    LOGDEBUG("start done");

  }

  void Renderer::setCameraRotation(const Vec3& rotation) {
    cameraRotationByMatrix = false;
    this->cameraRotationXYZ = rotation;
    this->cameraTransformation = rotate(Mat4(1.0f), -rotation.z, Vec3(0.0f, 0.0f, 1.0f)) *
      rotate(Mat4(1.0f), -rotation.x, Vec3(1.0f, 0.0f, 0.0f)) *
      rotate(Mat4(1.0f), -rotation.y, Vec3(0.0f, 1.0f, 0.0f));
  }

  void Renderer::rotateCamera(const Vec3& rotation) {
    if (cameraRotationByMatrix) {
      throw std::runtime_error("Attempted x, y, z representation camera rotation, while having set the initial rotation by matrix.");
    }
    else {
      this->cameraRotationXYZ += rotation;
      this->cameraTransformation = rotate(Mat4(1.0f), -this->cameraRotationXYZ.z, Vec3(0.0f, 0.0f, 1.0f)) *
        rotate(Mat4(1.0f), -this->cameraRotationXYZ.x, Vec3(1.0f, 0.0f, 0.0f)) *
        rotate(Mat4(1.0f), -this->cameraRotationXYZ.y, Vec3(0.0f, 1.0f, 0.0f));
    }
  }

  void Renderer::setCameraTransformation(const Mat4& rotation) {
    this->cameraTransformation = inverse(rotation);
    cameraRotationByMatrix = true;
    cameraRotationXYZ = Vec3(0.0f);
  }

  const Vec3 Renderer::getCameraOrientation() const {
    auto orientationVec4 = inverse(this->cameraTransformation) * Vec4(0.0f, 0.0f, -1.0f, 1.0f);
    return Vec3(orientationVec4.x, orientationVec4.y, orientationVec4.z);
  }

  const Mat4 Renderer::getCameraRotation() const {
    return inverse(this->cameraTransformation);
  }

  const Vec3 Renderer::getCameraRotationXYZ() const {
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
    for (auto idFacePair : fontFaces) {
      FT_Done_Face(idFacePair.second);
    }


    FT_Done_FreeType(library);

    stop();


    windowing.terminate();

  }

  void Renderer::stop() {

    glDeleteFramebuffers(1, &depthMapFramebuffer);
    depthMapFramebuffer = 0;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);


    glActiveTexture(GL_TEXTURE0 + 1);

    glBindTexture(GL_TEXTURE_2D, 0);

    glDeleteTextures(0, &depthMapTexture);

    for (auto it = textures.begin();
      it != textures.end(); ++it) {
      LOGDEBUG("Deleting texture " + it->first);
      glDeleteTextures(1, &it->second);
    }
    textures.clear();

    if (!noShaders) {
      glUseProgram(0);

      glDeleteVertexArrays(1, &vao);
      glBindVertexArray(0);

    }

    if (shaderProgram != 0) {
      glDeleteProgram(shaderProgram);
    }

  }

  void Renderer::generateTexture(const std::string& name, const Image& image) {
    LOGDEBUG("Sending image to GPU, dimensions " + std::to_string(image.getWidth()) +
      ", " + std::to_string(image.getHeight()));
    this->generateTexture(name, image.getData(), image.getWidth(),
      image.getHeight(), true);
  }

  void Renderer::generateTexture(const std::string& name, const std::string& text,
    const Vec3& colour, const int fontSize,
    const std::string& fontPath,
    const bool replace) {

    Vec3i icolour(colour.x * 255, colour.y * 255, colour.z * 255);

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

    textMemory.resize(4 * width * height);
    memset(&textMemory[0], 0, 4 * width * height);

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

            auto colourAlpha = Vec4i(icolour, 0);

            colourAlpha.w = slot->bitmap.buffer[row * slot->bitmap.width + col];

            auto pos = 4 * (maxTop -
              slot->bitmap_top
              + row) * static_cast<size_t>(width) +
              4 *
              (static_cast<size_t>(slot->bitmap_left) +
                static_cast<size_t>(col))
              + totalAdvance;

            textMemory[pos] = colourAlpha.x;
            textMemory[pos + 1] = colourAlpha.y;
            textMemory[pos + 2] = colourAlpha.z;
            textMemory[pos + 3] = colourAlpha.w;

          }
        }
      }
      totalAdvance += 4 * static_cast<unsigned long>(slot->advance.x / 64);
    }
    generateTexture(name, &textMemory[0], static_cast<unsigned long>(width), static_cast<unsigned long>(height), replace);
  }

  void Renderer::deleteTexture(const std::string& name) {
    auto nameTexturePair = textures.find(name);
    glActiveTexture(GL_TEXTURE0);
    if (nameTexturePair != textures.end()) {
      glBindTexture(GL_TEXTURE_2D, 0);
      glDeleteTextures(1, &(nameTexturePair->second));
      textures.erase(name);
    }
  }

  void Renderer::createRectangle(Model& rect,
    const Vec3& topLeft,
    const Vec3& bottomRight) {

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

    rect.indexDataByteSize = 6 * sizeof(uint16_t);

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

  void Renderer::render(Model& model, const Vec3& position, const Vec3& rotation,
    const Vec4& colour, const std::string& textureName, const uint64_t currentPose,
    const bool perspective) {

    this->render(model, position, rotate(Mat4(1.0f), rotation.y, Vec3(0.0f, 1.0f, 0.0f)) *
      rotate(Mat4(1.0f), rotation.x, Vec3(1.0f, 0.0f, 0.0f)) *
      rotate(Mat4(1.0f), rotation.z, Vec3(0.0f, 0.0f, 1.0f)),
      colour, textureName, currentPose, perspective);

  }

  void Renderer::render(Model& model, const Vec3& position, const Vec3& rotation,
    const std::string& textureName, const uint64_t currentPose) {

    this->render(model, position, rotate(Mat4(1.0f), rotation.y, Vec3(0.0f, 1.0f, 0.0f)) *
      rotate(Mat4(1.0f), rotation.x, Vec3(1.0f, 0.0f, 0.0f)) *
      rotate(Mat4(1.0f), rotation.z, Vec3(0.0f, 0.0f, 1.0f)),
      textureName, currentPose);

  }

  void Renderer::renderTuple(std::tuple< Model*, Vec3, Mat4, Vec4, std::string, bool, uint64_t> tuple) {

    Model* model = std::get<0>(tuple);
    Vec3 offset = std::get<1>(tuple);
    Mat4 rotation = std::get<2>(tuple);
    Vec4 colour = std::get<3>(tuple);
    std::string textureName = std::get<4>(tuple);
    bool perspective = std::get<5>(tuple);
    uint64_t currentPose = std::get<6>(tuple);

    if (!perspective && !renderingDepthMap) {
      glClear(GL_DEPTH_BUFFER_BIT);
    }

    glUseProgram(shaderProgram);

    GLint bufSize = 0;
    glBindBuffer(GL_ARRAY_BUFFER, model->positionBufferObjectId);
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufSize);
    // Flush invalid operation error (this is normal when the model has not
    // been loaded into the GPU).
    while (glGetError() == GL_INVALID_OPERATION);

    bool alreadyInGPU = bufSize > 0;

    if (!alreadyInGPU) {
      glGenBuffers(1, &model->indexBufferObjectId);
      glGenBuffers(1, &model->positionBufferObjectId);
      glGenBuffers(1, &model->normalsBufferObjectId);
      glGenBuffers(1, &model->uvBufferObjectId);
      glGenBuffers(1, &model->jointBufferObjectId);
      glGenBuffers(1, &model->weightBufferObjectId);
    }

    // Vertices
    glBindBuffer(GL_ARRAY_BUFFER, model->positionBufferObjectId);
    if (!alreadyInGPU) {
      glBufferData(GL_ARRAY_BUFFER,
        model->vertexDataByteSize,
        model->vertexData.data(),
        GL_STATIC_DRAW);
    }

    // Vertex indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->indexBufferObjectId);
    if (!alreadyInGPU) {
      glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        model->indexDataByteSize,
        model->indexData.data(),
        GL_STATIC_DRAW);
    }

    glEnableVertexAttribArray(attrib_position);
    glVertexAttribPointer(attrib_position, 4, GL_FLOAT, GL_FALSE, 0, 0);

    // Normals

    glBindBuffer(GL_ARRAY_BUFFER, model->normalsBufferObjectId);

    if (!alreadyInGPU) {
      if (model->normalsDataByteSize > 0) {
        glBufferData(GL_ARRAY_BUFFER,
          model->normalsDataByteSize,
          model->normalsData.data(),
          GL_STATIC_DRAW);
      }
      else {
        // The normals buffer is created with 0 values if the corresponding
        // data does not exist, when MacOS was supported this helped avoid
        // EXC_BAD_ACCESS errors.
        size_t ns = (model->vertexDataByteSize / 4) * 3;
        std::unique_ptr<char[]> data = std::make_unique<char[]>(ns);
        glBufferData(GL_ARRAY_BUFFER,
          ns,
          &data[0],
          GL_STATIC_DRAW);
      }
    }

    glEnableVertexAttribArray(attrib_normal);
    glVertexAttribPointer(attrib_normal, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    if (model->jointDataByteSize != 0) {
      glBindBuffer(GL_ARRAY_BUFFER, model->jointBufferObjectId);
      if (!alreadyInGPU) {
        glBufferData(GL_ARRAY_BUFFER,
          model->jointDataByteSize,
          model->jointData.data(),
          GL_STATIC_DRAW);
      }

      glEnableVertexAttribArray(attrib_joint);
      glVertexAttribIPointer(attrib_joint, 4, GL_UNSIGNED_BYTE, 0, 0);

    }

    if (model->weightDataByteSize != 0) {
      glBindBuffer(GL_ARRAY_BUFFER, model->weightBufferObjectId);
      if (!alreadyInGPU) {
        glBufferData(GL_ARRAY_BUFFER,
          model->weightDataByteSize,
          model->weightData.data(),
          GL_STATIC_DRAW);
      }

      glEnableVertexAttribArray(attrib_weight);
      glVertexAttribPointer(attrib_weight, 4, GL_FLOAT, GL_FALSE, 0, 0);

    }

    // Find the colour uniform
    GLint colourUniform = glGetUniformLocation(shaderProgram, "modelColour");

    if (textureName != "") {

      // "Disable" colour since there is a texture
       Vec4 col0; // (initialised with all 0s by default)
      glUniform4fv(colourUniform, 1,
        Value_ptr(col0));

      bindTexture(textureName);

      // UV Coordinates

      glBindBuffer(GL_ARRAY_BUFFER, model->uvBufferObjectId);

      if (!alreadyInGPU) {
        glBufferData(GL_ARRAY_BUFFER,
          model->textureCoordsDataByteSize,
          model->textureCoordsData.data(),
          GL_STATIC_DRAW);
      }

      glEnableVertexAttribArray(attrib_uv);
      glVertexAttribPointer(attrib_uv, 2, GL_FLOAT, GL_FALSE, 0, 0);
    }
    else {
      // If there is no texture, use the given colour
      glUniform4fv(colourUniform, 1, Value_ptr(colour));
      bindTexture("blank");
    }


    glActiveTexture(GL_TEXTURE0 + 1);

    if (!renderingDepthMap) {

      glBindTexture(GL_TEXTURE_2D, depthMapTexture);

    }
    else {
      glBindTexture(GL_TEXTURE_2D, 0);
    }

    setWorldDetails(perspective);

    transform(*model, offset, rotation, currentPose);



    // Draw
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDrawElements(GL_TRIANGLES,
      static_cast<GLsizei>(model->indexData.size()),
      GL_UNSIGNED_SHORT, 0);

    glDisableVertexAttribArray(attrib_position);
    glDisableVertexAttribArray(attrib_normal);
    if (model->jointDataByteSize != 0) glDisableVertexAttribArray(attrib_joint);
    if (model->weightDataByteSize != 0) glDisableVertexAttribArray(attrib_weight);
    if (textureName != "") glDisableVertexAttribArray(attrib_uv);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glUseProgram(0);
  }

  void Renderer::render(Model& model, const Vec3& position,
    const Mat4& rotation,
    const Vec4& colour,
    const std::string& textureName,
    const uint64_t currentPose,
    const bool perspective) {

    renderList.push_back(std::tuple<Model*, Vec3, Mat4, Vec4, std::string, bool, uint64_t>{&model, position, rotation, colour, textureName, perspective, currentPose});

  }

  void Renderer::render(Model& model, const Vec3& position,
    const Mat4& rotation,
    const std::string& textureName,
    const uint64_t currentPose) {
    this->render(model, position, rotation, Vec4(0.0f, 0.0f, 0.0f, 0.0f),
      textureName, currentPose);
  }

  void Renderer::render(Model& model, const std::string& textureName, const uint64_t currentPose,
    const bool perspective) {
    this->render(model, Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 0.0f), Vec4(0.0f, 0.0f, 0.0f, 0.0f),
      textureName, currentPose, perspective);
  }

  void Renderer::render(Model& model, const Vec4& colour, const uint64_t currentPose,
    const bool perspective) {
    this->render(model, Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 0.0f), colour,
      "", currentPose, perspective);
  }

  void Renderer::render(SceneObject& sceneObject,
    const Vec4& colour) {
    this->render(sceneObject.getModel(), sceneObject.position,
      sceneObject.transformation, colour, "", sceneObject.getCurrentPose());
  }

  void Renderer::render(SceneObject& sceneObject,
    const std::string& textureName) {
    this->render(sceneObject.getModel(), sceneObject.position,
      sceneObject.transformation, Vec4(0.0f, 0.0f, 0.0f, 0.0f),
      textureName, sceneObject.getCurrentPose());
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
    for (auto m : sceneObject.models) {
      clearBuffers(*m);
    }
  }

  void Renderer::setBackgroundColour(const Vec4& colour) {
    clearColour = colour;
    glClearColor(clearColour.x, clearColour.y, clearColour.z, clearColour.w);

  }

  void Renderer::swapBuffers() {

    lightSpaceMatrix = Mat4(0);

    if (shadowsActive) {
      glViewport(0, 0, depthMapTextureWidth, depthMapTextureHeight);
      glBindFramebuffer(GL_FRAMEBUFFER, depthMapFramebuffer);
      glClear(GL_DEPTH_BUFFER_BIT);

      // Store "normal" camera position and transformation (rotation)
      auto tmpCamTr = cameraTransformation;
      auto tmpCamPos = cameraPosition;

      // Position camera at 0. Position (translation) will be stored with transformation.
      cameraPosition = Vec3(0);

      cameraTransformation = shadowCamTransformation;

      // Render in orthographic mode on depth map framebuffer, only the models that are to be drawn using perspective
      // (Orthographically rendered models will not produce shadows as they are mostly used for messages and interface
      // components)
      renderingDepthMap = true;
      //glCullFace(GL_FRONT); // Avoid peter panning (but creates worse quality shadows)

      for (auto& tuple : renderList) {
        if (std::get<5>(tuple)) {
          auto tmpt = tuple;
          if (std::get<0>(tuple)->noShadow) {
            continue;
          }
          std::get<5>(tmpt) = false;
          renderTuple(tmpt);
        }
      }
      renderingDepthMap = false;
      glCullFace(GL_BACK); // Back to normal culling (after avoiding peter panning)

      // Store the light space camera transformation to be used during normal rendering to position shadows
      lightSpaceMatrix = cameraTransformation;

      // Return to "normal" camera rotation and position
      cameraTransformation = tmpCamTr;
      cameraPosition = tmpCamPos;

      glBindFramebuffer(GL_FRAMEBUFFER, origFramebuffer);

      glViewport(0, 0, static_cast<GLsizei>(windowing.realWindowWidth),
		 static_cast<GLsizei>(windowing.realWindowHeight));
    }

    for (const auto& tuple : renderList) {
      renderTuple(tuple);
    }
    renderList.clear();

#ifdef _WIN32
    if (screenCapture) {

      captureScreen();
    }
#endif

    windowing.swapBuffers();

    clearScreen();

  }

  GLFWwindow* Renderer::getWindow()
  {
      return windowing.getWindow();
  }

#ifdef _WIN32

  void Renderer::captureScreen() {

    GLint encoding = 0;
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_BACK_LEFT, GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING, &encoding);
    checkForOpenGLErrors("detecting back buffer encoding", false);
    switch (encoding) {
    case GL_LINEAR:
      LOGDEBUG("Capturing screen, GL_BACK buffer encoding detected is GL_LINEAR.");
      break;
    case GL_SRGB:
      LOGDEBUG("Capturing screen, GL_BACK buffer encoding detected is GL_SRGB.");
      break;
    default:
      LOGDEBUG("Capturing sceen, detected unforeseen encoding: " + std::to_string(encoding));
    }

    auto imgSizeRGBA = 4 * windowing.realWindowWidth * windowing.realWindowHeight;
    GLubyte* pixelsRGBA = new GLubyte[imgSizeRGBA];
    memset(pixelsRGBA, 255, imgSizeRGBA);
    auto imgSize = 3 * windowing.realWindowWidth * windowing.realWindowHeight;
    GLubyte* pixels = new GLubyte[imgSize];

    glReadBuffer(GL_BACK);

    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    checkForOpenGLErrors("setting pack alignment", true);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    checkForOpenGLErrors("setting unpack alignment", true);
    glReadPixels(0, 0, windowing.realWindowWidth, windowing.realWindowHeight, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, pixelsRGBA);
    checkForOpenGLErrors("capturing screen", true);

    uint32_t idxRGBA = 0, idxRGB = 0;

    // The colour format for GL_UNSIGNED_INT_8_8_8_8 is ABGR
    // See https://www.khronos.org/opengl/wiki/Pixel_Transfer#Pixel_layout

    while (idxRGBA < imgSizeRGBA) {
      pixels[idxRGB] = pixelsRGBA[idxRGBA + 2];
      ++idxRGB;
      pixels[idxRGB] = pixelsRGBA[idxRGBA + 3];
      ++idxRGB;
      pixels[idxRGB] = pixelsRGBA[idxRGBA + 1];
      ++idxRGB;
      idxRGBA += 4;
    }

    BITMAPINFOHEADER biheader;
    memset(&biheader, 0, sizeof(BITMAPINFOHEADER));
    biheader.biSize = sizeof(BITMAPINFOHEADER);
    biheader.biWidth = windowing.realWindowWidth;
    biheader.biHeight = windowing.realWindowHeight;
    biheader.biPlanes = 1;
    biheader.biBitCount = 24;
    biheader.biCompression = BI_RGB;
    biheader.biSizeImage = 0;
    biheader.biClrUsed = 0;
    biheader.biClrImportant = 0;

    BITMAPINFO biinfo;
    memset(&biinfo, 0, sizeof(BITMAPINFO));
    biinfo.bmiHeader = biheader;

    HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, sizeof(BITMAPINFO) + imgSize);
    if (mem != NULL) {
      HWND win = windowing.getWin32Window();
      if (OpenClipboard(win)) {
        EmptyClipboard();
        void* memLocked = GlobalLock(mem);
        if (memLocked) {
          memcpy(reinterpret_cast<char*>(memLocked), &biinfo, sizeof(BITMAPINFO));
          memcpy(reinterpret_cast<char*>(memLocked) + sizeof(BITMAPINFO), pixels, imgSize);
          GlobalUnlock(mem);
          SetClipboardData(CF_DIB, mem);
          CloseClipboard();
        }
      }
      else {
        GlobalFree(mem);
      }
    }

    delete[] pixels;
    screenCapture = false;

  }

#endif

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
