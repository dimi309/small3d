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

namespace small3d {

  static void error_callback(int error, const char* description)
  {
    LOGERROR(std::string(description));
  }

  struct uboWorld {
    glm::mat4 perspectiveMatrix;
    glm::vec3 lightDirection;
  };

  struct uboOrientation {
    glm::vec3 offset;
    glm::mat4x4 xRotationMatrix;
    glm::mat4x4 yRotationMatrix;
    glm::mat4x4 zRotationMatrix;
  };

  struct uboCamera {
    glm::vec3 position;
    glm::mat4x4 xRotationMatrix;
    glm::mat4x4 yRotationMatrix;
    glm::mat4x4 zRotationMatrix;
  };

  struct uboColour {
    glm::vec4 colour;
  };

  struct uboLight {
    float intensity;
  };
  
  static std::string openglErrorToString(GLenum error);

  std::string Renderer::loadShaderFromFile(const std::string fileLocation)
    const {
    initLogger();
    std::string shaderSource = "";
    std::ifstream file(fileLocation.c_str());
    std::string line;
    if (file.is_open()) {
      while (std::getline(file, line)) {
        shaderSource += line + "\n";
      }
    }
    return shaderSource;
  }

  GLuint Renderer::compileShader(const std::string shaderSourceFile,
				 const uint32_t shaderType) const {
    GLuint shader = glCreateShader(shaderType);
    std::string shaderSource = this->loadShaderFromFile(shaderSourceFile);
    if (shaderSource.length() == 0) {
      throw std::runtime_error("Shader source file '" + shaderSourceFile +
			       "' is empty or not found.");
    }
    const char *shaderSourceChars = shaderSource.c_str();
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

    uint32_t infoLogCharLength = (uint32_t)infoLogLength + (uint32_t)1;

    GLchar *infoLog = new GLchar[infoLogCharLength];
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

    GLchar *infoLog = new GLchar[infoLogLength + 1];
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
      std::string glewVersion = reinterpret_cast<char *>
	(const_cast<GLubyte*>(glewGetString(GLEW_VERSION)));
      LOGDEBUG("Using GLEW version " + glewVersion);
    }

    checkForOpenGLErrors("initialising GLEW", false);

    LOGINFO("OpenGL version: " +
	    std::string(reinterpret_cast<char *>
			(const_cast<GLubyte*>(glGetString(GL_VERSION)))));

  }

  void Renderer::checkForOpenGLErrors(const std::string when, const bool abort)
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

  void Renderer::positionNextObject(const glm::vec3 offset,
				    const glm::vec3 rotation) {

    uboOrientation orientation;
    memset(&orientation, 0, sizeof(uboOrientation));

    orientation.xRotationMatrix = glm::rotate(glm::mat4x4(1.0f), rotation.x,
					      glm::vec3(-1.0f, 0.0f, 0.0f));
    orientation.yRotationMatrix = glm::rotate(glm::mat4x4(1.0f), rotation.y,
					      glm::vec3(0.0f, -1.0f, 0.0f));
    orientation.zRotationMatrix = glm::rotate(glm::mat4x4(1.0f), rotation.z,
					      glm::vec3(0.0f, 0.0f, -1.0f));
    orientation.offset = offset;


    if (renderOrientation == 0) {
      GLuint orientationIndex = glGetUniformBlockIndex(perspectiveProgram,
						       "uboOrientation");
      glUniformBlockBinding(perspectiveProgram, orientationIndex, 1);
      glGenBuffers(1, &renderOrientation);
      
    }

    glBindBuffer(GL_UNIFORM_BUFFER, renderOrientation);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(uboOrientation), &orientation, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, renderOrientation);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    

    /*GLint xRotationMatrixUniform = glGetUniformLocation(perspectiveProgram,
      "xRotationMatrix");
      glUniformMatrix4fv(xRotationMatrixUniform, 1, GL_TRUE,
      glm::value_ptr(glm::rotate(glm::mat4x4(1.0f), rotation.x,
      glm::vec3(-1.0f, 0.0f, 0.0f)))
      );

      GLint yRotationMatrixUniform = glGetUniformLocation(perspectiveProgram,
      "yRotationMatrix");
      glUniformMatrix4fv(yRotationMatrixUniform, 1, GL_TRUE,
      glm::value_ptr(glm::rotate(glm::mat4x4(1.0f), rotation.y,
      glm::vec3(0.0f, -1.0f, 0.0f)))
      );

      GLint zRotationMatrixUniform = glGetUniformLocation(perspectiveProgram,
      "zRotationMatrix");
      glUniformMatrix4fv(zRotationMatrixUniform, 1, GL_TRUE,
      glm::value_ptr(glm::rotate(glm::mat4x4(1.0f), rotation.z,
      glm::vec3(0.0f, 0.0f, -1.0f)))
      );

      GLint offsetUniform = glGetUniformLocation(perspectiveProgram, "offset");
      glUniform3fv(offsetUniform, 1, glm::value_ptr(offset));*/
  }


  void Renderer::positionCamera() {

    uboCamera camera;
    memset(&camera, 0, sizeof(uboCamera));
    camera.position = cameraPosition;
    camera.xRotationMatrix = glm::rotate(glm::mat4x4(1.0f), -cameraRotation.x,
					 glm::vec3(-1.0f, 0.0f, 0.0f));
    camera.yRotationMatrix = glm::rotate(glm::mat4x4(1.0f), -cameraRotation.y,
					 glm::vec3(0.0f, -1.0f, 0.0f));
    camera.zRotationMatrix = glm::rotate(glm::mat4x4(1.0f), -cameraRotation.z,
					 glm::vec3(0.0f, 0.0f, -1.0f));
  
    if (cameraOrientation == 0) {
      GLuint orientationIndex = glGetUniformBlockIndex(perspectiveProgram, "uboCamera");
      glUniformBlockBinding(perspectiveProgram, orientationIndex, 2);
      glGenBuffers(1, &cameraOrientation);
    }

    
    glBindBuffer(GL_UNIFORM_BUFFER, cameraOrientation);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(uboCamera), &camera, GL_DYNAMIC_DRAW);
    checkForOpenGLErrors("camera data", true);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, cameraOrientation);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    /*
    GLint xCameraRotationMatrixUniform =
      glGetUniformLocation(perspectiveProgram, "xCameraRotationMatrix");
    GLint yCameraRotationMatrixUniform =
      glGetUniformLocation(perspectiveProgram, "yCameraRotationMatrix");
    GLint zCameraRotationMatrixUniform =
      glGetUniformLocation(perspectiveProgram, "zCameraRotationMatrix");


    glUniformMatrix4fv(xCameraRotationMatrixUniform, 1, GL_TRUE, 
		       glm::value_ptr(glm::rotate(glm::mat4x4(1.0f),
						  -cameraRotation.x,
						  glm::vec3(-1.0f, 0.0f, 0.0f)))
		       );
    glUniformMatrix4fv(yCameraRotationMatrixUniform, 1, GL_TRUE, 
		       glm::value_ptr(glm::rotate(glm::mat4x4(1.0f),
						  -cameraRotation.y,
						  glm::vec3(0.0f, -1.0f, 0.0f)))
		       );
    glUniformMatrix4fv(zCameraRotationMatrixUniform, 1, GL_TRUE, 
		       glm::value_ptr(glm::rotate(glm::mat4x4(1.0f),
						  -cameraRotation.z,
						  glm::vec3(0.0f, 0.0f, -1.0f)))
		       );

    // Camera position
    GLint cameraPositionUniform = glGetUniformLocation(perspectiveProgram,
						       "cameraPosition");
    glUniform3fv(cameraPositionUniform, 1, glm::value_ptr(cameraPosition));
    */
  }

  GLuint Renderer::getTextureHandle(const std::string name) const {
    GLuint handle = 0;
    auto nameTexturePair = textures.find(name);
    if (nameTexturePair != textures.end()) {
      handle = nameTexturePair->second;
    }
    return handle;
  }

  GLuint Renderer::generateTexture(const std::string name, const float* data,
				   const unsigned long width,
				   const unsigned long height) {

    GLuint textureHandle;
    glGenTextures(1, &textureHandle);
    glBindTexture(GL_TEXTURE_2D, textureHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

    GLint internalFormat = GL_RGBA32F;

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, GL_RGBA,
		 GL_FLOAT, data);

    textures.insert(make_pair(name, textureHandle));

    return textureHandle;
  }

  void Renderer::init(const int width, const int height,
		      const std::string windowTitle,
		      const std::string shadersPath) {

    realScreenWidth = width;
    realScreenHeight = height;

    this->initWindow(realScreenWidth, realScreenHeight, windowTitle);

    this->initOpenGL();

    glViewport(0, 0, static_cast<GLsizei>(realScreenWidth),
	       static_cast<GLsizei>(realScreenHeight));

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRange(zNear + zOffsetFromCamera, zFar);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLuint vertexShader = compileShader(shadersPath +
					"perspectiveMatrixLightedShader.vert",
					GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(shadersPath + "textureShader.frag",
					  GL_FRAGMENT_SHADER);

    perspectiveProgram = glCreateProgram();
    glAttachShader(perspectiveProgram, vertexShader);
    glAttachShader(perspectiveProgram, fragmentShader);

    glLinkProgram(perspectiveProgram);

    GLint status;
    glGetProgramiv(perspectiveProgram, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
      throw std::runtime_error("Failed to link program:\n" +
			       this->getProgramInfoLog(perspectiveProgram));
    }
    else {
      LOGDEBUG("Linked main rendering program successfully");

      glUseProgram(perspectiveProgram);

      // Perspective set for the first time here

      setPerspectiveAndLight();
      
      /*      GLint perspectiveMatrixUniform =
	glGetUniformLocation(perspectiveProgram, "perspectiveMatrix");

      
      glUniformMatrix4fv(perspectiveMatrixUniform, 1, GL_FALSE,
			 perspectiveMatrix);
      */
      glUseProgram(0);
    }
    glDetachShader(perspectiveProgram, vertexShader);
    glDetachShader(perspectiveProgram, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);

    // Program (with shaders) for orthographic rendering for text

    GLuint simpleVertexShader = compileShader(shadersPath +
					      "simpleShader.vert",
					      GL_VERTEX_SHADER);
    GLuint simpleFragmentShader = compileShader(shadersPath +
						"simpleShader.frag",
						GL_FRAGMENT_SHADER);

    orthographicProgram = glCreateProgram();
    glAttachShader(orthographicProgram, simpleVertexShader);
    glAttachShader(orthographicProgram, simpleFragmentShader);

    glLinkProgram(orthographicProgram);

    glGetProgramiv(orthographicProgram, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
      throw std::runtime_error("Failed to link program:\n" +
			       this->getProgramInfoLog(orthographicProgram));
    }
    else {
      LOGDEBUG("Linked orthographic rendering program successfully");
    }
    glDetachShader(orthographicProgram, simpleVertexShader);
    glDetachShader(orthographicProgram, simpleFragmentShader);
    glDeleteShader(simpleVertexShader);
    glDeleteShader(simpleFragmentShader);
    glUseProgram(0);
  }

  void Renderer::initWindow(int &width, int &height,
			    const std::string windowTitle) {

    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
      throw std::runtime_error("Unable to initialise GLFW");
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    bool fullScreen = false;

    GLFWmonitor *monitor = nullptr; // If NOT null, a full-screen window will
    // be created.

    if ((width == 0 && height != 0) || (width != 0 && height == 0)) {
      throw std::runtime_error("Screen width and height both have to be equal "
			       "or not equal to zero at the same time.");
    }
    else if (width == 0) {

      fullScreen = true;

      monitor = glfwGetPrimaryMonitor();

      const GLFWvidmode* mode = glfwGetVideoMode(monitor);
      width = mode->width;
      height = mode->height;

      LOGINFO("Detected screen width " + intToStr(width) + " and height " +
	      intToStr(height));
    }

    window = glfwCreateWindow(width, height, windowTitle.c_str(), monitor,
			      nullptr);
    if (!window) {
      throw std::runtime_error("Unable to create GLFW window");
    }

    glfwMakeContextCurrent(window);

  }

  void Renderer::setPerspectiveAndLight() {
    uboWorld world;
    float tmpmat4[16];
    memset(&tmpmat4, 0, 16 * sizeof(float));
    tmpmat4[0] = frustumScale;
    tmpmat4[5] = frustumScale * realScreenWidth / realScreenHeight;
    tmpmat4[10] = (zNear + zFar) / (zNear - zFar);
    tmpmat4[14] = 2.0f * zNear * zFar / (zNear - zFar);
    tmpmat4[11] = zOffsetFromCamera;
    world.perspectiveMatrix = glm::make_mat4(tmpmat4);
    world.lightDirection = lightDirection;

    if (worldDetails == 0) {
      GLuint worldIndex = glGetUniformBlockIndex(perspectiveProgram, "uboWorld");
      glUniformBlockBinding(perspectiveProgram, worldIndex, 0);
      checkForOpenGLErrors("world data binding", true);
      glGenBuffers(1, &worldDetails);
    }
    
    glBindBuffer(GL_UNIFORM_BUFFER, worldDetails);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(uboWorld), &world, GL_DYNAMIC_DRAW);
    checkForOpenGLErrors("world data", true);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, worldDetails);
    checkForOpenGLErrors("world data bind base", true);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    uboLight light;
    memset(&light, 0, sizeof(uboLight));
    light.intensity = lightIntensity;

    if (lightUboId == 0) {
      GLuint lightUboIndex = glGetUniformBlockIndex(perspectiveProgram,
        "uboLight");
      glUniformBlockBinding(perspectiveProgram, lightUboIndex, 5);
      glGenBuffers(1, &lightUboId);
    }
    
    glBindBuffer(GL_UNIFORM_BUFFER, lightUboId);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(uboLight), &light, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 5, lightUboId);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

  }

  Renderer::Renderer() {
    window = 0;
    perspectiveProgram = 0;
    orthographicProgram = 0;
    noShaders = false;
    lightDirection = glm::vec3(0.0f, 0.9f, 0.2f);
    cameraPosition = glm::vec3(0, 0, 0);
    cameraRotation = glm::vec3(0, 0, 0);
  
  }
  
  Renderer::Renderer(const std::string windowTitle, const int width,
		     const int height, const float frustumScale,
		     const float zNear, const float zFar,
		     const float zOffsetFromCamera,
		     const std::string shadersPath) {
    
    window = 0;
    perspectiveProgram = 0;
    orthographicProgram = 0;
    noShaders = false;
    lightDirection = glm::vec3(0.0f, 0.9f, 0.2f);
    cameraPosition = glm::vec3(0, 0, 0);
    cameraRotation = glm::vec3(0, 0, 0);
    this->zNear = zNear;
    this->zFar = zFar;
    this->frustumScale = frustumScale;
    this->zOffsetFromCamera = zOffsetFromCamera;
    
    init(width, height, windowTitle, shadersPath);
    
    FT_Error ftError = FT_Init_FreeType( &library );
    
    if(ftError != 0) {
      throw std::runtime_error("Unable to initialise font system");
    }

    // Generate VAO
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
  }
  
  Renderer& Renderer::getInstance(const std::string windowTitle, 
				  const int width, const int height,
				  const float frustumScale,
				  const float zNear, const float zFar,
				  const float zOffsetFromCamera, 
				  const std::string shadersPath) {
    
    static Renderer instance(windowTitle, width, height, frustumScale, zNear,
			     zFar, zOffsetFromCamera, shadersPath);
    return instance;
  }
  
  Renderer::~Renderer() {
    LOGDEBUG("Renderer destructor running");
    for (auto it = textures.begin();
	 it != textures.end(); ++it) {
      LOGDEBUG("Deleting texture " + it->first);
      glDeleteTextures(1, &it->second);
    }
    
    for(auto idFacePair : fontFaces) {
      FT_Done_Face(idFacePair.second);
    }
    
    FT_Done_FreeType(library);
    
    if (!noShaders) {
      glUseProgram(0);

      glDeleteVertexArrays(1, &vao);
      glBindVertexArray(0);

    }
    
    if (orthographicProgram != 0) {
      glDeleteProgram(orthographicProgram);
    }
    
    if (perspectiveProgram != 0) {
      glDeleteProgram(perspectiveProgram);
    }
    
    glfwTerminate();
  }
  
  GLFWwindow* Renderer::getWindow() const{
    return window;
  }

  void Renderer::generateTexture(const std::string name, const Image image) {
    this->generateTexture(name, image.getData(), image.getWidth(),
			  image.getHeight());
  }

  void Renderer::generateTexture(const std::string name, const std::string text,
		       const glm::vec3 colour, const int fontSize,
		       const std::string fontPath) {
    
    std::string faceId = intToStr(fontSize) + fontPath;
    
    auto idFacePair = fontFaces.find(faceId);
    FT_Face face;
    FT_Error error;
    
    if (idFacePair == fontFaces.end()) {
      std::string faceFullPath = fontPath;
      LOGDEBUG("Loading font from " + faceFullPath);
      error = FT_New_Face(library, faceFullPath.c_str(), 0, &face);
      if (error != 0) {
        throw std::runtime_error("Failed to load font from " + faceFullPath);
      }
      else{
        LOGDEBUG("Font loaded successfully");
        fontFaces.insert(make_pair(faceId, face));
      }
    } else {
      face = idFacePair->second;
    }
    
    // Multiplying by 64 to convert to 26.6 fractional points. Using 100dpi.
    error = FT_Set_Char_Size(face, 64 * fontSize, 0, 100, 0);
    
    if (error != 0) {
      throw std::runtime_error("Failed to set font size.");
    }
    
    unsigned long width = 0, maxTop = 0, height = 0;
    
    // Figure out bitmap dimensions
    for(const char &c: text) {
      error = FT_Load_Char(face, (FT_ULong) c, FT_LOAD_RENDER);
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
    
    for(const char &c: text) {
      error = FT_Load_Char(face, (FT_ULong) c, FT_LOAD_RENDER);
      if (error != 0) {
        throw std::runtime_error("Failed to load character glyph.");
      }
      
      FT_GlyphSlot slot = face->glyph;
      
      if (slot->bitmap.width * slot->bitmap.rows > 0) {
        for (int row = 0; row < static_cast<int>(slot->bitmap.rows); ++row){
          for (int col = 0; col < static_cast<int>(slot->bitmap.width); ++col) {

	    glm::vec4 colourAlpha = glm::vec4(colour, 0.0f);

	    colourAlpha.a =
	      floorf(100.0f * (static_cast<float>
			       (slot->bitmap.buffer[row * slot->bitmap.width +
						    col]) /
			       255.0f) + 0.5f) / 100.0f;
	    
            memcpy(&textMemory[4 * (maxTop -
				    static_cast<unsigned long>(slot->bitmap_top)
				    + static_cast<unsigned long>(row)) * width +
			       4 *
			       (static_cast<unsigned long>(slot->bitmap_left) +
				    static_cast<unsigned long>(col)) 
			       + totalAdvance],
		   glm::value_ptr(colourAlpha),
		   4 * sizeof(float));
          }
        }
      }
      totalAdvance += 4 * static_cast<unsigned long>(slot->advance.x / 64);
    }
    generateTexture(name, &textMemory[0], width, height);
  }
  
  void Renderer::deleteTexture(const std::string name) {
    auto nameTexturePair = textures.find(name);
    
    if (nameTexturePair != textures.end()) {
      glDeleteTextures(1, &(nameTexturePair->second));
      textures.erase(name);
    }
  }
  
  void Renderer::renderRectangle(const std::string textureName,
				 const glm::vec3 topLeft,
				 const glm::vec3 bottomRight,
				 const bool perspective,
				 const glm::vec4 colour) {
    
    float vertices[16] = {
      bottomRight.x, bottomRight.y, bottomRight.z, 1.0f,
      bottomRight.x, topLeft.y, topLeft.z, 1.0f,
      topLeft.x, topLeft.y, topLeft.z, 1.0f,
      topLeft.x, bottomRight.y, bottomRight.z, 1.0f
    };
    
    glUseProgram(perspective ? perspectiveProgram : orthographicProgram);
    
    GLuint boxBuffer = 0;
    glGenBuffers(1, &boxBuffer);

    // Vertices
    glBindBuffer(GL_ARRAY_BUFFER, boxBuffer);
    glBufferData(GL_ARRAY_BUFFER,
		 sizeof(float) * 16,
		 &vertices[0],
		 GL_STATIC_DRAW);
    
    unsigned int vertexIndexes[6] = {
      0, 1, 2,
      2, 3, 0
    };
    
    GLuint indexBufferObject = 0;
    
    glGenBuffers(1, &indexBufferObject);

    // Vertex indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 6,
		 vertexIndexes, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    //glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    uboColour colourStruct;
    memset(&colourStruct, 0, sizeof(colourStruct));
    colourStruct.colour = colour;

    
    if (colourUboId == 0) {
      GLuint colourUboIndex = glGetUniformBlockIndex(perspective ? perspectiveProgram :
        orthographicProgram, "uboColour");
      glUniformBlockBinding(perspective ? perspectiveProgram :
        orthographicProgram, colourUboIndex, perspective ? 4 : 1);
      glGenBuffers(1, &colourUboId);
     
    }
    
    glBindBuffer(GL_UNIFORM_BUFFER, colourUboId);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(uboColour), &colourStruct, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, perspective ? 4 : 1, colourUboId);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

   /* GLint colourUniform =
      glGetUniformLocation( perspective ? perspectiveProgram :
			    orthographicProgram, "colour");
    glUniform4fv(colourUniform, 1, glm::value_ptr(colour));*/

    GLuint coordBuffer = 0;
    
    if (colour == glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)) {
      
      GLuint textureHandle = getTextureHandle(textureName);

      if (textureHandle == 0) {
        throw std::runtime_error("Texture " + textureName +
				 " has not been generated");
      }

      glBindTexture(GL_TEXTURE_2D, textureHandle);

      float textureCoords[8] = {
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,
        0.0f, 1.0f
	};

      glGenBuffers(1, &coordBuffer);
      glBindBuffer(GL_ARRAY_BUFFER, coordBuffer);
      glBufferData(GL_ARRAY_BUFFER,
		   sizeof(float) * 8,
		   textureCoords,
		   GL_STATIC_DRAW);
      glEnableVertexAttribArray(perspective? 2 : 1);
      glVertexAttribPointer(perspective? 2 : 1, 2, GL_FLOAT, GL_FALSE, 0, 0);
      
    }

    if (perspective) {

      // Lighting
      setPerspectiveAndLight();
      /*      GLint lightDirectionUniform = glGetUniformLocation(perspectiveProgram,
							 "lightDirection");
      glUniform3fv(lightDirectionUniform, 1,
		   glm::value_ptr(lightDirection));
      

      GLint lightIntensityUniform = glGetUniformLocation(perspectiveProgram,
							 "lightIntensity");
      glUniform1f(lightIntensityUniform, lightIntensity);
      */
      positionNextObject(glm::vec3(0.0f, 0.0f, 0.0f),
			 glm::vec3(0.0f, 0.0f, 0.0f));
      positionCamera();
    }
    
    glDrawElements(GL_TRIANGLES,
		   6, GL_UNSIGNED_INT, 0);
    
    glDeleteBuffers(1, &indexBufferObject);
    glDeleteBuffers(1, &boxBuffer);
    if (colour == glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)) {
      glDeleteBuffers(1, &coordBuffer);
      glDisableVertexAttribArray(perspective? 2 : 1);
      glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    glDisableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    checkForOpenGLErrors("rendering rectangle", true);
  }

  void Renderer::renderRectangle(const glm::vec4 colour,
				 const glm::vec3 topLeft,
				 const glm::vec3 bottomRight,
				 const bool perspective) {
    this->renderRectangle("", topLeft, bottomRight, perspective, colour);
  }
  
  void Renderer::render(Model &model, const glm::vec3 offset,
			const glm::vec3 rotation, 
			const glm::vec4 colour,
			const std::string textureName) {
    
    glUseProgram(perspectiveProgram);
    
    bool alreadyInGPU = model.positionBufferObjectId != 0;
    
    if (!alreadyInGPU) {
      glGenBuffers(1, &model.indexBufferObjectId);
      glGenBuffers(1, &model.positionBufferObjectId);
      glGenBuffers(1, &model.normalsBufferObjectId);
      glGenBuffers(1, &model.uvBufferObjectId);
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

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    
    // Normals
    glBindBuffer(GL_ARRAY_BUFFER, model.normalsBufferObjectId);
    if (!alreadyInGPU) {
      glBufferData(GL_ARRAY_BUFFER,
		   model.normalsDataByteSize,
		   model.normalsData.data(),
		   GL_STATIC_DRAW);
    }
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
    
    
    // Find the colour uniform
    //GLint colourUniform = glGetUniformLocation(perspectiveProgram, "colour");

    if (colourUboId == 0) {
      
      GLuint colourUboIndex = glGetUniformBlockIndex(perspectiveProgram, "uboColour");
      glUniformBlockBinding(perspectiveProgram, colourUboIndex, 4);
      glGenBuffers(1, &colourUboId);
     
    }

    uboColour colourStruct;
    memset(&colourStruct, 0, sizeof(uboColour));
    
    if (textureName != "") {
      
      // "Disable" colour since there is a texture
      //glUniform4fv(colourUniform, 1,
		  // glm::value_ptr(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)));
      colourStruct.colour = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
      
      glBindBuffer(GL_UNIFORM_BUFFER, colourUboId);
      glBufferData(GL_UNIFORM_BUFFER, sizeof(uboColour), &colourStruct, GL_DYNAMIC_DRAW);
      glBindBufferBase(GL_UNIFORM_BUFFER, 4, colourUboId);
      glBindBuffer(GL_UNIFORM_BUFFER, 0);
      
      GLuint textureHandle = this->getTextureHandle(textureName);

      if (textureHandle == 0) {
        throw std::runtime_error("Texture " + textureName +
				 "has not been generated");
      }
      
      glBindTexture(GL_TEXTURE_2D, textureHandle);
      
      // UV Coordinates
      
      glBindBuffer(GL_ARRAY_BUFFER, model.uvBufferObjectId);
      
      if (!alreadyInGPU) {
        glBufferData(GL_ARRAY_BUFFER,
		     model.textureCoordsDataByteSize,
		     model.textureCoordsData.data(),
		     GL_STATIC_DRAW);
      }
      
      glEnableVertexAttribArray(2);
      glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
      
    }
    else {
      // If there is no texture, use the given colour
      //glUniform4fv(colourUniform, 1, glm::value_ptr(colour));
      colourStruct.colour = colour;
      
      glBindBuffer(GL_UNIFORM_BUFFER, colourUboId);
      glBufferData(GL_UNIFORM_BUFFER, sizeof(uboColour), &colourStruct, GL_DYNAMIC_DRAW);
      glBindBufferBase(GL_UNIFORM_BUFFER, 4, colourUboId);
      glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
    
    
    setPerspectiveAndLight();
    /*GLint lightDirectionUniform = glGetUniformLocation(perspectiveProgram,
						       "lightDirection");
    glUniform3fv(lightDirectionUniform, 1,
		 glm::value_ptr(lightDirection));
    
    GLint lightIntensityUniform = glGetUniformLocation(perspectiveProgram,
						       "lightIntensity");
    glUniform1f(lightIntensityUniform, lightIntensity);
    */
    

    positionNextObject(offset, rotation);
    
    positionCamera();
    
    // Throw an exception if there was an error in OpenGL, during
    // any of the above.
    checkForOpenGLErrors("rendering model", true);
    
    // Draw
    glDrawElements(GL_TRIANGLES,
		   static_cast<GLsizei>(model.indexData.size()),
		   GL_UNSIGNED_INT, 0);
    
    // Clear stuff
    if (textureName != "") {
      glDisableVertexAttribArray(2);
    }
    
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    glUseProgram(0);
    
  }

  void Renderer::render(Model &model, const glm::vec3 offset,
			const glm::vec3 rotation,
			const std::string textureName) {
    this->render(model, offset, rotation, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
		 textureName);
  }

  void Renderer::render(SceneObject &sceneObject,
			const glm::vec4 colour) {
    this->render(sceneObject.getModel(), sceneObject.offset,
		 sceneObject.rotation, colour, "");
  }

  void Renderer::render(SceneObject &sceneObject,
			const std::string textureName) {
    this->render(sceneObject.getModel(), sceneObject.offset,
		 sceneObject.rotation, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
		 textureName);
  }
  
  void Renderer::write(const std::string text, const glm::vec3 colour,
		       const glm::vec2 topLeft, const glm::vec2 bottomRight,
		       const int fontSize, std::string fontPath) {

    std::string textureName = intToStr(fontSize) + "text_" + text;
    
    this->generateTexture(textureName, text, colour, fontSize, fontPath);
    
    renderRectangle(textureName, glm::vec3(topLeft.x, topLeft.y, -0.5f),
		    glm::vec3(bottomRight.x, bottomRight.y, -0.5f));
    
    deleteTexture(textureName);
  }
  
  void Renderer::clearBuffers(Model &model) const {
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
  
  void Renderer::clearScreen() const {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }
  
  void Renderer::clearScreen(const glm::vec4 colour) const {
    glClearColor(colour.r, colour.g, colour.b, colour.a); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }
  
  void Renderer::swapBuffers() const {
    glfwSwapBuffers(window);
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
