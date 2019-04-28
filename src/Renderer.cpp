/*
 * Renderer.cpp
 *
 *  Created on: 2014/10/19
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

extern "C" {
#include "vkzos.h"
}
#include "Renderer.hpp"
#include <stdexcept>
#include <fstream>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

static VkVertexInputBindingDescription bd[3];
static VkVertexInputAttributeDescription ad[3];

namespace small3d {

  void error_callback(int error, const char* description)
  {
    LOGERROR(std::string(description));
  }

  struct uboWorld {
    glm::mat4 perspectiveMatrix;
    glm::vec3 lightDirection;
    float padding;
  };

  struct uboOrientation {
    glm::mat4x4 xRotationMatrix;
    glm::mat4x4 yRotationMatrix;
    glm::mat4x4 zRotationMatrix;
    glm::vec3 offset;
    float padding;
  };

  struct uboCamera {
    glm::mat4x4 xRotationMatrix;
    glm::mat4x4 yRotationMatrix;
    glm::mat4x4 zRotationMatrix;
    glm::vec3 position;
  };

  struct uboColour {
    glm::vec4 colour;
  };

  struct uboLight {
    float intensity;
  };

  int setInputStateCallback(VkPipelineVertexInputStateCreateInfo* inputStateCreateInfo) {
    LOGDEBUG("setInputStateCallback called.");
    
    memset(bd, 0, 3 * sizeof(VkVertexInputBindingDescription));

    bd[0].binding = 0;
    bd[0].stride = 4 * sizeof(float);

    bd[1].binding = 1;
    bd[1].stride = 3 * sizeof(float);

    bd[2].binding = 2;
    bd[2].stride = 2 * sizeof(float);
        
    memset(ad, 0, 3 * sizeof(VkVertexInputAttributeDescription));

    ad[0].binding = 0;
    ad[0].location = 0;
    ad[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    ad[0].offset = 0;

    ad[1].binding = 1;
    ad[1].location = 1;
    ad[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    ad[1].offset = 0;

    ad[2].binding = 2;
    ad[2].location = 2;
    ad[2].format = VK_FORMAT_R32G32_SFLOAT;
    ad[2].offset = 0;
    
    inputStateCreateInfo->vertexBindingDescriptionCount = 1;
    inputStateCreateInfo->vertexAttributeDescriptionCount = 3;
    inputStateCreateInfo->pVertexBindingDescriptions = bd;
    inputStateCreateInfo->pVertexAttributeDescriptions = ad;

    return 1;
  }

  int setPipelineLayoutCallback(VkPipelineLayoutCreateInfo* pipelineLayoutCreateInfo) {
    LOGDEBUG("setPipelineLayoutCallback called.");

    return 1;
  }

  void Renderer::initVulkan() {

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::string requiredExtensions = "GLFW required extensions (";
    requiredExtensions += intToStr(glfwExtensionCount) + ")";
    LOGDEBUG(requiredExtensions);

    for (uint32_t n = 0; n < glfwExtensionCount; n++) {
      LOGDEBUG(glfwExtensions[n]);
    }
    printf("\n\r");

    if (!vkz_create_instance(windowTitle.c_str(), glfwExtensions,
      glfwExtensionCount)) {
      throw std::runtime_error("Failed to create Vulkan instance.");
    }

    if (glfwCreateWindowSurface(vkz_instance, window, NULL, &vkz_surface) !=
      VK_SUCCESS) {
      throw std::runtime_error("Could not create surface.");
    }

    if (!vkz_init()) {
      throw std::runtime_error("Could not initialise Vulkan.");
    }

    if (!vkz_create_swapchain(realScreenWidth, realScreenHeight, 1)) {
      throw std::runtime_error("Failed to create swapchain.");
    }

    if (!vkz_create_depth_image()) {
      throw std::runtime_error("Failed to create depth image.");
    }
    createDescriptorPool();
    allocateDescriptorSets();

    /*glewExperimental = GL_TRUE;

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
      */
  }
  void Renderer::createDescriptorPool() {
    if (!descriptorPoolCreated) {

      VkDescriptorPoolSize ps[6]; 
      memset(&ps, 0, 6 * sizeof(VkDescriptorPoolSize));

      ps[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      ps[0].descriptorCount = vkz_swapchain_image_count;

      ps[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      ps[1].descriptorCount = vkz_swapchain_image_count;

      ps[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      ps[2].descriptorCount = vkz_swapchain_image_count;

      ps[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      ps[3].descriptorCount = vkz_swapchain_image_count;

      ps[4].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      ps[4].descriptorCount = vkz_swapchain_image_count;

      ps[5].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      ps[5].descriptorCount = vkz_swapchain_image_count;

      VkDescriptorPoolCreateInfo dpci;
      memset(&dpci, 0, sizeof(VkDescriptorPoolCreateInfo));
      dpci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
      dpci.poolSizeCount = 6;
      dpci.pPoolSizes = ps;
      dpci.maxSets = vkz_swapchain_image_count;

      if (vkCreateDescriptorPool(vkz_logical_device, &dpci, NULL,
        &descriptorPool) != VK_SUCCESS) {
        LOGDEBUG("Failed to create descriptor pool.");
      }
      else {
        descriptorPoolCreated = true;
        LOGDEBUG("Created descriptor pool.");
      }
    }
    
  }

  void Renderer::allocateDescriptorSets() {
    LOGDEBUG("Allocating descriptor sets.");
    VkDescriptorSetLayoutBinding dslb[6];
    memset(dslb, 0, 6 * sizeof(VkDescriptorSetLayoutBinding));

    dslb[0].binding = 0;
    dslb[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dslb[0].descriptorCount = 1;
    dslb[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    dslb[0].pImmutableSamplers = NULL;

    dslb[1].binding = 1;
    dslb[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dslb[1].descriptorCount = 1;
    dslb[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    dslb[1].pImmutableSamplers = NULL;

    dslb[2].binding = 2;
    dslb[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dslb[2].descriptorCount = 1;
    dslb[2].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    dslb[2].pImmutableSamplers = NULL;

    dslb[3].binding = 3;
    dslb[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    dslb[3].descriptorCount = 1;
    dslb[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dslb[3].pImmutableSamplers = NULL;

    dslb[4].binding = 4;
    dslb[4].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dslb[4].descriptorCount = 1;
    dslb[4].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dslb[4].pImmutableSamplers = NULL;

    dslb[5].binding = 5;
    dslb[5].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dslb[5].descriptorCount = 1;
    dslb[5].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dslb[5].pImmutableSamplers = NULL;

    VkDescriptorSetLayoutCreateInfo dslci;
    memset(&dslci, 0, sizeof(VkDescriptorSetLayoutCreateInfo));
    dslci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    dslci.bindingCount = 6;
    dslci.pBindings = dslb;

    if (vkCreateDescriptorSetLayout(vkz_logical_device, &dslci, NULL,
      &descriptorSetLayout) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create descriptor set layout.");
    }
    else {
      LOGDEBUG("Created descriptor set layout.");
    }

    std::vector<VkDescriptorSetLayout> dslo(vkz_swapchain_image_count); 

    for (size_t i = 0; i < vkz_swapchain_image_count; i++) {
      dslo[i] = descriptorSetLayout;
      LOGDEBUG("Set descriptor set layout for swapchain image " + intToStr((const int)i));
    }

    VkDescriptorSetAllocateInfo dsai;
    memset(&dsai, 0, sizeof(VkDescriptorSetAllocateInfo));
    dsai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    dsai.descriptorPool = descriptorPool;
    dsai.descriptorSetCount = vkz_swapchain_image_count;
    dsai.pSetLayouts = &dslo[0];

    descriptorSets = std::vector<VkDescriptorSet>(vkz_swapchain_image_count);
    VkResult allocResult = vkAllocateDescriptorSets(vkz_logical_device, &dsai,
      &descriptorSets[0]);
    if ( allocResult != VK_SUCCESS) {
      std::string errortxt = "Failed to allocate descriptor sets.";
      if (allocResult == VK_ERROR_OUT_OF_POOL_MEMORY) {
        errortxt += " (out of pool memory)";
      }
      LOGDEBUG(errortxt);
    }
    
  }

  void Renderer::positionNextObject(const glm::vec3 offset,
    const glm::vec3 rotation) {

    uboOrientation orientation;
    memset(&orientation, 0, sizeof(uboOrientation));

    orientation.xRotationMatrix = glm::transpose(glm::rotate(glm::mat4x4(1.0f), rotation.x,
      glm::vec3(1.0f, 0.0f, 0.0f)));
    orientation.yRotationMatrix = glm::transpose(glm::rotate(glm::mat4x4(1.0f), rotation.y,
      glm::vec3(0.0f, 1.0f, 0.0f)));
    orientation.zRotationMatrix = glm::transpose(glm::rotate(glm::mat4x4(1.0f), rotation.z,
      glm::vec3(0.0f, 0.0f, 1.0f)));
    orientation.offset = offset;
    
    uint32_t renderOrientationSize = (3 * 16 + 3) * sizeof(float);

    if (renderOrientationBuffers.size() == 0) {
      renderOrientationBuffers = std::vector<VkBuffer>(vkz_swapchain_image_count);
      renderOrientationBufferMemories = std::vector<VkDeviceMemory>(vkz_swapchain_image_count);
      
      for (size_t i = 0; i < vkz_swapchain_image_count; i++) {
        vkz_create_buffer(&renderOrientationBuffers[i], VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
          renderOrientationSize,
          &renderOrientationBufferMemories[i],
          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
      }
      /*GLuint orientationIndex = glGetUniformBlockIndex(perspectiveProgram,
        "uboOrientation");
      glUniformBlockBinding(perspectiveProgram, orientationIndex, 1);
      glGenBuffers(1, &renderOrientation);*/
    }

    void* orientationData;
    vkMapMemory(vkz_logical_device, renderOrientationBufferMemories[currentSwapchainImageIndex],
      0, renderOrientationSize, 0, &orientationData);
    memcpy(orientationData, &orientation, renderOrientationSize);
    vkUnmapMemory(vkz_logical_device, renderOrientationBufferMemories[currentSwapchainImageIndex]);

    /*glBindBuffer(GL_UNIFORM_BUFFER, renderOrientation);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(uboOrientation), &orientation, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, renderOrientation);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);*/

  }

  void Renderer::positionCamera() {

    uboCamera camera;
    memset(&camera, 0, sizeof(uboCamera));
    camera.position = cameraPosition;
    camera.xRotationMatrix = glm::transpose(glm::rotate(glm::mat4x4(1.0f), cameraRotation.x,
      glm::vec3(-1.0f, 0.0f, 0.0f)));
    camera.yRotationMatrix = glm::transpose(glm::rotate(glm::mat4x4(1.0f), cameraRotation.y,
      glm::vec3(0.0f, -1.0f, 0.0f)));
    camera.zRotationMatrix = glm::transpose(glm::rotate(glm::mat4x4(1.0f), cameraRotation.z,
      glm::vec3(0.0f, 0.0f, -1.0f)));

    if (cameraOrientation == 0) {
      /*GLuint orientationIndex = glGetUniformBlockIndex(perspectiveProgram, "uboCamera");
      glUniformBlockBinding(perspectiveProgram, orientationIndex, 2);
      glGenBuffers(1, &cameraOrientation);*/
    }

    /* glBindBuffer(GL_UNIFORM_BUFFER, cameraOrientation);
     glBufferData(GL_UNIFORM_BUFFER, sizeof(uboCamera), &camera, GL_DYNAMIC_DRAW);
     glBindBufferBase(GL_UNIFORM_BUFFER, 2, cameraOrientation);
     glBindBuffer(GL_UNIFORM_BUFFER, 0);*/

  }

  uint32_t Renderer::getTextureHandle(const std::string name) const {
    GLuint handle = 0;
    auto nameTexturePair = textures.find(name);
    if (nameTexturePair != textures.end()) {
      handle = nameTexturePair->second;
    }
    return handle;
  }

  uint32_t Renderer::generateTexture(const std::string name, const float* data,
    const unsigned long width,
    const unsigned long height) {

    uint32_t textureHandle = 0;
    /*glGenTextures(1, &textureHandle);
    glBindTexture(GL_TEXTURE_2D, textureHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

    GLint internalFormat = GL_RGBA32F;

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, GL_RGBA,
      GL_FLOAT, data);

    textures.insert(make_pair(name, textureHandle));*/

    return textureHandle;
  }

  void Renderer::init(const int width, const int height,
    const std::string shadersPath) {

    realScreenWidth = width;
    realScreenHeight = height;

    this->initWindow(realScreenWidth, realScreenHeight);

    this->initVulkan();

    std::string vertexShaderPath = shadersPath +
      "perspectiveMatrixLightedShader.spv";
    std::string fragmentShaderPath = shadersPath +
      "textureShader.spv";
    
    vkz_create_pipeline(vertexShaderPath.c_str(), fragmentShaderPath.c_str(),
      setInputStateCallback, setPipelineLayoutCallback, &perspectivePipelineIndex);

    //glViewport(0, 0, static_cast<GLsizei>(realScreenWidth),
    //  static_cast<GLsizei>(realScreenHeight));

    //glEnable(GL_DEPTH_TEST);
    //glDepthMask(GL_TRUE);
    //glDepthFunc(GL_LEQUAL);
    //glDepthRange(zNear + zOffsetFromCamera, zFar);

    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //GLuint vertexShader = compileShader(shadersPath +
    //  "perspectiveMatrixLightedShader.vert",
    //  GL_VERTEX_SHADER);
    //GLuint fragmentShader = compileShader(shadersPath + "textureShader.frag",
    //  GL_FRAGMENT_SHADER);

    //perspectiveProgram = glCreateProgram();
    //glAttachShader(perspectiveProgram, vertexShader);
    //glAttachShader(perspectiveProgram, fragmentShader);

    //glLinkProgram(perspectiveProgram);

    //GLint status;
    //glGetProgramiv(perspectiveProgram, GL_LINK_STATUS, &status);
    //if (status == GL_FALSE) {
    //  throw std::runtime_error("Failed to link program:\n" +
    //    this->getProgramInfoLog(perspectiveProgram));
    //}
    //else {
    //  LOGDEBUG("Linked main rendering program successfully");

    //  glUseProgram(perspectiveProgram);

    //  setPerspectiveAndLight();

    //  glUseProgram(0);
    //}
    //glDetachShader(perspectiveProgram, vertexShader);
    //glDetachShader(perspectiveProgram, fragmentShader);
    //glDeleteShader(vertexShader);
    //glDeleteShader(fragmentShader);

    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    //glFrontFace(GL_CCW);

    //glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    //glClearDepth(1.0f);

    //// Program (with shaders) for orthographic rendering for text

    //GLuint simpleVertexShader = compileShader(shadersPath +
    //  "simpleShader.vert",
    //  GL_VERTEX_SHADER);
    //GLuint simpleFragmentShader = compileShader(shadersPath +
    //  "simpleShader.frag",
    //  GL_FRAGMENT_SHADER);

    //orthographicProgram = glCreateProgram();
    //glAttachShader(orthographicProgram, simpleVertexShader);
    //glAttachShader(orthographicProgram, simpleFragmentShader);

    //glLinkProgram(orthographicProgram);

    //glGetProgramiv(orthographicProgram, GL_LINK_STATUS, &status);
    //if (status == GL_FALSE) {
    //  throw std::runtime_error("Failed to link program:\n" +
    //    this->getProgramInfoLog(orthographicProgram));
    //}
    //else {
    //  LOGDEBUG("Linked orthographic rendering program successfully");
    //}
    //glDetachShader(orthographicProgram, simpleVertexShader);
    //glDetachShader(orthographicProgram, simpleFragmentShader);
    //glDeleteShader(simpleVertexShader);
    //glDeleteShader(simpleFragmentShader);
    //glUseProgram(0);
  }

  void Renderer::initWindow(int& width, int& height) {

    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
      throw std::runtime_error("Unable to initialise GLFW");
    }

    /*glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);*/

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    bool fullScreen = false;

    GLFWmonitor* monitor = nullptr; // If NOT null, a full-screen window will
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

    //glfwMakeContextCurrent(window);

  }

  void Renderer::setPerspectiveAndLight() {
    uboWorld world;
    memset(&world, 0, sizeof(uboWorld));
    float tmpmat4[16];
    memset(&tmpmat4, 0, 16 * sizeof(float));
    tmpmat4[0] = frustumScale;
    tmpmat4[5] = frustumScale * realScreenWidth / realScreenHeight;
    tmpmat4[10] = (zNear + zFar) / (zNear - zFar);
    tmpmat4[11] = 2.0f * zNear * zFar / (zNear - zFar);
    tmpmat4[14] = zOffsetFromCamera;
    world.perspectiveMatrix = glm::make_mat4(tmpmat4);
    world.lightDirection = lightDirection;

    if (worldDetails == 0) {
      /*GLuint worldIndex = glGetUniformBlockIndex(perspectiveProgram, "uboWorld");
      glUniformBlockBinding(perspectiveProgram, worldIndex, 0);
      glGenBuffers(1, &worldDetails);*/
    }

    /* glBindBuffer(GL_UNIFORM_BUFFER, worldDetails);
     glBufferData(GL_UNIFORM_BUFFER, sizeof(uboWorld), &world, GL_DYNAMIC_DRAW);
     glBindBufferBase(GL_UNIFORM_BUFFER, 0, worldDetails);
     glBindBuffer(GL_UNIFORM_BUFFER, 0);*/

    uboLight light;
    memset(&light, 0, sizeof(uboLight));
    light.intensity = lightIntensity;

    if (lightUboId == 0) {
      /*GLuint lightUboIndex = glGetUniformBlockIndex(perspectiveProgram,
        "uboLight");
      glUniformBlockBinding(perspectiveProgram, lightUboIndex, 5);
      glGenBuffers(1, &lightUboId);*/
    }

    /*glBindBuffer(GL_UNIFORM_BUFFER, lightUboId);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(uboLight), &light, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 5, lightUboId);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);*/

  }
  void Renderer::bindTexture(std::string name, bool perspective) {
    /*GLuint textureHandle = getTextureHandle(name);

    if (textureHandle == 0) {
      throw std::runtime_error("Texture " + name +
        " has not been generated");
    }*/

    /*if (perspective)
      glActiveTexture(GL_TEXTURE0);
    else
      glActiveTexture(GL_TEXTURE1);

    glBindTexture(GL_TEXTURE_2D, textureHandle);
    GLint loc = glGetUniformLocation(perspective ? perspectiveProgram : orthographicProgram, "textureImage");

    glUniform1i(loc, perspective ? 0 : 1);*/

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
    this->windowTitle = windowTitle;

    init(width, height, shadersPath);

    FT_Error ftError = FT_Init_FreeType(&library);

    if (ftError != 0) {
      throw std::runtime_error("Unable to initialise font system");
    }

    // Generate VAO
    /*glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);*/
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
      //glDeleteTextures(1, &it->second);
    }

    for (auto idFacePair : fontFaces) {
      FT_Done_Face(idFacePair.second);
    }

    FT_Done_FreeType(library);

    /*if (!noShaders) {
      glUseProgram(0);

      glDeleteVertexArrays(1, &vao);
      glBindVertexArray(0);

    }

    if (orthographicProgram != 0) {
      glDeleteProgram(orthographicProgram);
    }

    if (perspectiveProgram != 0) {
      glDeleteProgram(perspectiveProgram);
    }*/

    if (perspectivePipelineIndex != 100) {
      vkz_destroy_pipeline(perspectivePipelineIndex);
    }

    vkDestroyDescriptorSetLayout(vkz_logical_device,
      descriptorSetLayout, NULL);

    if (descriptorPoolCreated) {
      vkDestroyDescriptorPool(vkz_logical_device, descriptorPool, NULL);
    }

    if (renderOrientationBuffers.size() > 0) {
      LOGDEBUG("Destroying render orientation buffers.");
      for (size_t i = 0; i < vkz_swapchain_image_count; i++) {
        vkz_destroy_buffer(renderOrientationBuffers[i], renderOrientationBufferMemories[i]);
      }
    }

    vkz_destroy_depth_image();
    vkz_destroy_swapchain();
    vkz_shutdown();

    //glfwDestroyWindow(window);

    // On linux this causes a segmentation fault
#ifndef __linux__
    //glfwTerminate();
#endif
  }

  GLFWwindow* Renderer::getWindow() const {
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

    unsigned long width = 0, maxTop = 0, height = 0;

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
      //glDeleteTextures(1, &(nameTexturePair->second));
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

    //glUseProgram(perspective ? perspectiveProgram : orthographicProgram);

    //GLuint boxBuffer = 0;
    //glGenBuffers(1, &boxBuffer);

    //// Vertices
    //glBindBuffer(GL_ARRAY_BUFFER, boxBuffer);
    //glBufferData(GL_ARRAY_BUFFER,
    //  sizeof(float) * 16,
    //  &vertices[0],
    //  GL_STATIC_DRAW);

    unsigned int vertexIndexes[6] = {
      0, 1, 2,
      2, 3, 0
    };

    //GLuint indexBufferObject = 0;

    //glGenBuffers(1, &indexBufferObject);

    //// Vertex indices
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 6,
    //  vertexIndexes, GL_STATIC_DRAW);

    //glEnableVertexAttribArray(0);
    //glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);


    uboColour colourStruct;
    memset(&colourStruct, 0, sizeof(colourStruct));
    colourStruct.colour = colour;

    if (perspective) {
      if (perspColourUboId == 0) {
        /*GLuint colourUboIndex = glGetUniformBlockIndex(perspectiveProgram, "uboColour");
        glUniformBlockBinding(perspectiveProgram, colourUboIndex, 4);
        glGenBuffers(1, &perspColourUboId);*/

      }
    }
    else {
      if (orthoColourUboId == 0) {
        /*GLuint colourUboIndex = glGetUniformBlockIndex(orthographicProgram, "uboColour");
        glUniformBlockBinding(orthographicProgram, colourUboIndex, 1);
        glGenBuffers(1, &orthoColourUboId);*/

      }

    }

    /*glBindBuffer(GL_UNIFORM_BUFFER, perspective ? perspColourUboId : orthoColourUboId);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(uboColour), &colourStruct, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, perspective ? 4 : 1, perspective ? perspColourUboId : orthoColourUboId);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    GLuint coordBuffer = 0;*/

    if (colour == glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)) {

      bindTexture(textureName, perspective);

      float textureCoords[8] = {
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,
        0.0f, 1.0f
      };

      /*glGenBuffers(1, &coordBuffer);
      glBindBuffer(GL_ARRAY_BUFFER, coordBuffer);
      glBufferData(GL_ARRAY_BUFFER,
        sizeof(float) * 8,
        textureCoords,
        GL_STATIC_DRAW);
      glEnableVertexAttribArray(perspective ? 2 : 1);
      glVertexAttribPointer(perspective ? 2 : 1, 2, GL_FLOAT, GL_FALSE, 0, 0);*/

    }

    if (perspective) {

      setPerspectiveAndLight();

      positionNextObject(glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.0f));
      positionCamera();
    }

    /*glDrawElements(GL_TRIANGLES,
      6, GL_UNSIGNED_INT, 0);*/

      /*glDeleteBuffers(1, &indexBufferObject);
      glDeleteBuffers(1, &boxBuffer);*/
    if (colour == glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)) {
      /* glDeleteBuffers(1, &coordBuffer);
       glDisableVertexAttribArray(perspective ? 2 : 1);
       glBindTexture(GL_TEXTURE_2D, 0);*/
    }

    /*  glDisableVertexAttribArray(0);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);*/

  }

  void Renderer::renderRectangle(const glm::vec4 colour,
    const glm::vec3 topLeft,
    const glm::vec3 bottomRight,
    const bool perspective) {
    this->renderRectangle("", topLeft, bottomRight, perspective, colour);
  }

  void Renderer::render(Model & model, const glm::vec3 offset,
    const glm::vec3 rotation,
    const glm::vec4 colour,
    const std::string textureName) {

    /*glUseProgram(perspectiveProgram);*/

    bool alreadyInGPU = model.positionBufferObjectId != 0;

    if (!alreadyInGPU) {
      /* glGenBuffers(1, &model.indexBufferObjectId);
       glGenBuffers(1, &model.positionBufferObjectId);
       glGenBuffers(1, &model.normalsBufferObjectId);
       glGenBuffers(1, &model.uvBufferObjectId);*/
    }

    // Vertices
   /* glBindBuffer(GL_ARRAY_BUFFER, model.positionBufferObjectId);*/
    if (!alreadyInGPU) {
      /* glBufferData(GL_ARRAY_BUFFER,
         model.vertexDataByteSize,
         model.vertexData.data(),
         GL_STATIC_DRAW);*/
    }

    // Vertex indices
    /*glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.indexBufferObjectId);*/
    if (!alreadyInGPU) {
      /*glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        model.indexDataByteSize,
        model.indexData.data(),
        GL_STATIC_DRAW);*/
    }

    /* glEnableVertexAttribArray(0);
     glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
 */
 // Normals
 /*glBindBuffer(GL_ARRAY_BUFFER, model.normalsBufferObjectId);*/
    if (!alreadyInGPU) {
      /* glBufferData(GL_ARRAY_BUFFER,
         model.normalsDataByteSize,
         model.normalsData.data(),
         GL_STATIC_DRAW);*/
    }
    /*glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);*/

    if (perspColourUboId == 0) {

      /*GLuint colourUboIndex = glGetUniformBlockIndex(perspectiveProgram, "uboColour");
      glUniformBlockBinding(perspectiveProgram, colourUboIndex, 4);
      glGenBuffers(1, &perspColourUboId);*/

    }

    uboColour colourStruct;
    memset(&colourStruct, 0, sizeof(uboColour));

    if (textureName != "") {

      // "Disable" colour since there is a texture

      colourStruct.colour = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

      /*glBindBuffer(GL_UNIFORM_BUFFER, perspColourUboId);
      glBufferData(GL_UNIFORM_BUFFER, sizeof(uboColour), &colourStruct, GL_DYNAMIC_DRAW);
      glBindBufferBase(GL_UNIFORM_BUFFER, 4, perspColourUboId);
      glBindBuffer(GL_UNIFORM_BUFFER, 0);*/

      bindTexture(textureName, true);

      // UV Coordinates

     /* glBindBuffer(GL_ARRAY_BUFFER, model.uvBufferObjectId);*/

      if (!alreadyInGPU) {
        /* glBufferData(GL_ARRAY_BUFFER,
           model.textureCoordsDataByteSize,
           model.textureCoordsData.data(),
           GL_STATIC_DRAW);*/
      }

      /*  glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);*/

    }
    else {
      // If there is no texture, use the given colour

      colourStruct.colour = colour;

      /* glBindBuffer(GL_UNIFORM_BUFFER, perspColourUboId);
       glBufferData(GL_UNIFORM_BUFFER, sizeof(uboColour), &colourStruct, GL_DYNAMIC_DRAW);
       glBindBufferBase(GL_UNIFORM_BUFFER, 4, perspColourUboId);
       glBindBuffer(GL_UNIFORM_BUFFER, 0);*/
    }

    setPerspectiveAndLight();

    positionNextObject(offset, rotation);

    positionCamera();

    // Draw
    /*glDrawElements(GL_TRIANGLES,
      static_cast<GLsizei>(model.indexData.size()),
      GL_UNSIGNED_INT, 0);*/

      // Clear stuff
    if (textureName != "") {
      /*glDisableVertexAttribArray(2);*/
    }

    /*glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glUseProgram(0);*/

  }

  void Renderer::render(Model & model, const glm::vec3 offset,
    const glm::vec3 rotation,
    const std::string textureName) {
    this->render(model, offset, rotation, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
      textureName);
  }

  void Renderer::render(SceneObject & sceneObject,
    const glm::vec4 colour) {
    this->render(sceneObject.getModel(), sceneObject.offset,
      sceneObject.rotation, colour, "");
  }

  void Renderer::render(SceneObject & sceneObject,
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

  void Renderer::clearBuffers(Model & model) const {
    if (model.positionBufferObjectId != 0) {
      /* glDeleteBuffers(1, &model.positionBufferObjectId);*/
      model.positionBufferObjectId = 0;
    }

    if (model.indexBufferObjectId != 0) {
      /*glDeleteBuffers(1, &model.indexBufferObjectId);*/
      model.indexBufferObjectId = 0;
    }
    if (model.normalsBufferObjectId != 0) {
      /*glDeleteBuffers(1, &model.normalsBufferObjectId);*/
      model.normalsBufferObjectId = 0;
    }

    if (model.uvBufferObjectId != 0) {
      /*glDeleteBuffers(1, &model.uvBufferObjectId);*/
      model.uvBufferObjectId = 0;
    }
  }

  void Renderer::clearScreen() const {
    /*glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);*/
  }

  void Renderer::clearScreen(const glm::vec4 colour) const {
    /*glClearColor(colour.r, colour.g, colour.b, colour.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);*/
  }

  void Renderer::swapBuffers() const {
    /* glfwSwapBuffers(window);*/
  }

}
