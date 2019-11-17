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
#include <cstring>

#ifdef SMALL3D_IOS
#include "interop.h"
#endif

namespace small3d {

  void errorCallback(int error, const char* description)
  {
    LOGERROR(std::string(description));
  }

  /**
   * @brief Structure used to keep track of the uniform buffer that contains
   *        the direction of the light and the matrix used to add perspective to
   *        the scene. Used internally
   */
  struct UboWorld {
    glm::mat4 perspectiveMatrix;
    glm::vec3 lightDirection;
    float padding;
  };

  /**
   * @brief Structure used to keep track of the camera orientation uniform 
   *        buffer created on the GPU. Used internally
   */
  struct UboCamera {
    glm::mat4x4 xRotationMatrix;
    glm::mat4x4 yRotationMatrix;
    glm::mat4x4 zRotationMatrix;
    glm::vec3 position;
    float padding;
  };

  /**
   * @brief Structure used to keep track of the light intensity uniform buffer
   *        created on the GPU. Used internally
   */
  struct UboLight {
    float intensity;
  };

  std::vector<Model> Renderer::nextModelsToDraw;

  VkVertexInputBindingDescription Renderer::bd[3];
  VkVertexInputAttributeDescription Renderer::ad[3];
  VkDescriptorSetLayout Renderer::descriptorSetLayout;
  VkDescriptorSet Renderer::descriptorSet;
  VkDescriptorSetLayout Renderer::textureDescriptorSetLayout;
  VkDescriptorSetLayout Renderer::perspectiveLayouts[2];

  VkVertexInputBindingDescription Renderer::orthobd[2];
  VkVertexInputAttributeDescription Renderer::orthoad[2];
  VkDescriptorSetLayout Renderer::orthoDescriptorSetLayout;
  VkDescriptorSet Renderer::orthoDescriptorSet;
  VkDescriptorSetLayout Renderer::textureOrthoDescriptorSetLayout;
  VkDescriptorSetLayout Renderer::orthographicLayouts[2];

  int Renderer::realScreenWidth;
  int Renderer::realScreenHeight;

  void Renderer::framebufferSizeCallback(GLFWwindow* window, int width,
					 int height) {
    realScreenWidth = width;
    realScreenHeight = height;
    vkz_set_width_height(width, height);
    LOGDEBUG("New framebuffer dimensions " + intToStr(width) + " x " +
	     intToStr(height));
  }

  int Renderer::setInputStateCallback(VkPipelineVertexInputStateCreateInfo*
				      inputStateCreateInfo) {

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

    inputStateCreateInfo->vertexBindingDescriptionCount = 3;
    inputStateCreateInfo->vertexAttributeDescriptionCount = 3;
    inputStateCreateInfo->pVertexBindingDescriptions = bd;
    inputStateCreateInfo->pVertexAttributeDescriptions = ad;

    return 1;
  }

  int Renderer::setPipelineLayoutCallback(VkPipelineLayoutCreateInfo*
					  pipelineLayoutCreateInfo) {
    perspectiveLayouts[0] = descriptorSetLayout;
    perspectiveLayouts[1] = textureDescriptorSetLayout;
    pipelineLayoutCreateInfo->setLayoutCount = 2;
    pipelineLayoutCreateInfo->pSetLayouts = perspectiveLayouts;

    return 1;
  }

  int Renderer::bindBuffers(VkCommandBuffer commandBuffer, const Model& model) {
    VkBuffer vertexBuffers[3];
    vertexBuffers[0] = model.positionBuffer;
    vertexBuffers[1] = model.normalsBuffer;
    vertexBuffers[2] = model.uvBuffer;
    VkDeviceSize offsets[3];
    offsets[0] = 0;
    offsets[1] = 0;
    offsets[2] = 0;

    // Vertex buffer
    vkCmdBindVertexBuffers(commandBuffer, 0, 3, vertexBuffers, offsets);

    // Index buffer
    vkCmdBindIndexBuffer(commandBuffer, model.indexBuffer,
      0, VK_INDEX_TYPE_UINT32);
    return 1;
  }

  void Renderer::recordDrawCommand(VkCommandBuffer commandBuffer,
    VkPipelineLayout pipelineLayout, const Model& model,
    uint32_t swapchainImageIndex) {

    uint32_t dynamicOrientationOffset = model.orientationMemIndex *
      static_cast<uint32_t>(dynamicOrientationAlignment);

    uint32_t dynamicColourOffset = model.colourMemIndex *
      static_cast<uint32_t>(dynamicColourAlignment);

    const uint32_t dynamicOffsets[2] = { dynamicOrientationOffset,
					 dynamicColourOffset };

    const VkDescriptorSet descriptorSets[2] =
      { descriptorSet, getTextureHandle(model.textureName).descriptorSet };

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
      pipelineLayout, 0, 2,
      descriptorSets, 2, dynamicOffsets);

    vkCmdDrawIndexed(commandBuffer, (uint32_t)model.indexData.size(),
		     1, 0, 0, 0);


  }

  int Renderer::setOrthoInputStateCallback(VkPipelineVertexInputStateCreateInfo*
					   inputStateCreateInfo) {
    memset(orthobd, 0, 2 * sizeof(VkVertexInputBindingDescription));

    orthobd[0].binding = 0;
    orthobd[0].stride = 4 * sizeof(float);

    orthobd[1].binding = 1;
    orthobd[1].stride = 2 * sizeof(float);

    memset(orthoad, 0, 2 * sizeof(VkVertexInputAttributeDescription));

    orthoad[0].binding = 0;
    orthoad[0].location = 0;
    orthoad[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    orthoad[0].offset = 0;

    orthoad[1].binding = 1;
    orthoad[1].location = 1;
    orthoad[1].format = VK_FORMAT_R32G32_SFLOAT;
    orthoad[1].offset = 0;

    inputStateCreateInfo->vertexBindingDescriptionCount = 2;
    inputStateCreateInfo->vertexAttributeDescriptionCount = 2;
    inputStateCreateInfo->pVertexBindingDescriptions = orthobd;
    inputStateCreateInfo->pVertexAttributeDescriptions = orthoad;

    return 1;
  }

  int Renderer::setOrthoPipelineLayoutCallback(VkPipelineLayoutCreateInfo*
					       pipelineLayoutCreateInfo) {
    orthographicLayouts[0] = orthoDescriptorSetLayout;
    orthographicLayouts[1] = textureOrthoDescriptorSetLayout;
    pipelineLayoutCreateInfo->setLayoutCount = 2;
    pipelineLayoutCreateInfo->pSetLayouts = orthographicLayouts;
    return 1;
  }

  int Renderer::bindOrthoBuffers(VkCommandBuffer commandBuffer,
				 const Model& model) {
    VkBuffer vertexBuffers[2];
    vertexBuffers[0] = model.positionBuffer;
    vertexBuffers[1] = model.uvBuffer;
    VkDeviceSize offsets[2];
    offsets[0] = 0;
    offsets[1] = 0;

    // Vertex buffer
    vkCmdBindVertexBuffers(commandBuffer, 0, 2, vertexBuffers, offsets);

    // Index buffer
    vkCmdBindIndexBuffer(commandBuffer, model.indexBuffer,
      0, VK_INDEX_TYPE_UINT32);
    return 1;
  }

  void Renderer::recordOrthoDrawCommand(VkCommandBuffer commandBuffer,
    VkPipelineLayout pipelineLayout, const Model& model,
    uint32_t swapchainImageIndex) {

    uint32_t dynamicColourOffset = model.colourMemIndex *
      static_cast<uint32_t>(dynamicColourAlignment);

    const VkDescriptorSet descriptorSets[2] = { orthoDescriptorSet,
      getTextureHandle(model.textureName).orthoDescriptorSet };

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
      pipelineLayout, 0, 2,
      descriptorSets, 1, &dynamicColourOffset);

    vkCmdDrawIndexed(commandBuffer, (uint32_t)model.indexData.size(),
		     1, 0, 0, 0);

  }

  void Renderer::initVulkan() {
#if defined(__ANDROID__)
    const char *exts[2];

    exts[0] = VK_KHR_SURFACE_EXTENSION_NAME;
    exts[1] = VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;

    uint32_t num = 2;

    LOGDEBUG("Creating Vulkan instance...");

    if (!vkz_create_instance(windowTitle.c_str(), exts, num)) {
      throw std::runtime_error("Failed to create Vulkan instance.");
    }

    VkAndroidSurfaceCreateInfoKHR sci;
    sci.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    sci.pNext = nullptr;
    sci.flags = 0;
    sci.window = vkz_android_app->window;
    LOGDEBUG("Creating surface...");
    if (vkCreateAndroidSurfaceKHR(vkz_instance, &sci, nullptr, &vkz_surface) !=
      VK_SUCCESS) {
      throw std::runtime_error("Could not create surface.");
    }
#elif defined(SMALL3D_IOS)
    
    const char *exts[2];
    
    exts[0] = VK_KHR_SURFACE_EXTENSION_NAME;
    exts[1] = VK_MVK_IOS_SURFACE_EXTENSION_NAME;
    
    uint32_t num = 2;
    
    LOGDEBUG("Creating Vulkan instance...");

    if (!vkz_create_instance(windowTitle.c_str(), exts, num)) {
      throw std::runtime_error("Failed to create Vulkan instance.");
    }
    
    if (!create_ios_surface(vkz_instance, &vkz_surface)) {
      throw std::runtime_error("Could not create surface.");
    }
    
#else
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
#endif

    LOGDEBUG("Completing Vulkan initialisation...");

    if (!vkz_init()) {
      throw std::runtime_error("Could not initialise Vulkan.");
    }

    vkz_set_width_height(realScreenWidth, realScreenHeight);

    LOGDEBUG("Creating swapchain...");

    if (!vkz_create_swapchain(1)) {
      throw std::runtime_error("Failed to create swapchain.");
    }

    LOGDEBUG("Creating descriptor pool...");

    createDescriptorPool();
    allocateDescriptorSets();

    createOrthoDescriptorPool();
    allocateOrthoDescriptorSets();

  }

  void Renderer::createDescriptorPool() {
    if (!descriptorPoolCreated) {

      VkDescriptorPoolSize ps[6];

      memset(ps, 0, 6 * sizeof(VkDescriptorPoolSize));

      ps[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      ps[0].descriptorCount = vkz_swapchain_image_count;

      ps[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
      ps[1].descriptorCount = vkz_swapchain_image_count;

      ps[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      ps[2].descriptorCount = vkz_swapchain_image_count;

      // TODO: Review descriptor pool size
      ps[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      ps[3].descriptorCount = vkz_swapchain_image_count * 2 * maxObjectsPerPass;

      ps[4].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      ps[4].descriptorCount = vkz_swapchain_image_count;

      ps[5].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      ps[5].descriptorCount = vkz_swapchain_image_count;

      VkDescriptorPoolCreateInfo dpci;
      memset(&dpci, 0, sizeof(VkDescriptorPoolCreateInfo));
      dpci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
      dpci.poolSizeCount = 6;
      dpci.pPoolSizes = ps;
      dpci.maxSets = vkz_swapchain_image_count * 2 * maxObjectsPerPass;

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

  void Renderer::createOrthoDescriptorPool() {
    if (!orthoDescriptorPoolCreated) {

      VkDescriptorPoolSize ps[2];
      memset(ps, 0, 2 * sizeof(VkDescriptorPoolSize));

      ps[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      ps[0].descriptorCount = vkz_swapchain_image_count * 2 * maxObjectsPerPass;

      ps[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
      ps[1].descriptorCount = vkz_swapchain_image_count * 2 * maxObjectsPerPass;

      VkDescriptorPoolCreateInfo dpci;
      memset(&dpci, 0, sizeof(VkDescriptorPoolCreateInfo));
      dpci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
      dpci.poolSizeCount = 2;
      dpci.pPoolSizes = ps;
      dpci.maxSets = vkz_swapchain_image_count;

      if (vkCreateDescriptorPool(vkz_logical_device, &dpci, NULL,
        &orthoDescriptorPool) != VK_SUCCESS) {
        LOGDEBUG("Failed to create orthographic shaders descriptor pool.");
      }
      else {
        orthoDescriptorPoolCreated = true;
        LOGDEBUG("Created orthographic shaders descriptor pool.");
      }
    }
  }

  void Renderer::allocateDescriptorSets() {

    VkDescriptorSetLayoutBinding dslb[6];
    memset(dslb, 0, 5 * sizeof(VkDescriptorSetLayoutBinding));

    // perspectiveMatrixLightedShader - uboWorld
    dslb[0].binding = 0;
    dslb[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dslb[0].descriptorCount = 1;
    dslb[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    dslb[0].pImmutableSamplers = NULL;

    // perspectiveMatrixLightedShader - uboOrientation

    dslb[1].binding = 1;
    dslb[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    dslb[1].descriptorCount = 1;
    dslb[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    dslb[1].pImmutableSamplers = NULL;

    // perspectiveMatrixLightedShader - uboCamera
    dslb[2].binding = 2;
    dslb[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dslb[2].descriptorCount = 1;
    dslb[2].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    dslb[2].pImmutableSamplers = NULL;

    // textureShader - uboColour
    dslb[3].binding = 4;
    dslb[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    dslb[3].descriptorCount = 1;
    dslb[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dslb[3].pImmutableSamplers = NULL;

    // textureShader - uboLight
    dslb[4].binding = 5;
    dslb[4].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dslb[4].descriptorCount = 1;
    dslb[4].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dslb[4].pImmutableSamplers = NULL;

    VkDescriptorSetLayoutCreateInfo dslci;
    memset(&dslci, 0, sizeof(VkDescriptorSetLayoutCreateInfo));
    dslci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    dslci.bindingCount = 5;
    dslci.pBindings = dslb;

    if (vkCreateDescriptorSetLayout(vkz_logical_device, &dslci, NULL,
      &descriptorSetLayout) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create descriptor set layout.");
    }
    else {
      LOGDEBUG("Created descriptor set layout.");
    }

    VkDescriptorSetAllocateInfo dsai;
    memset(&dsai, 0, sizeof(VkDescriptorSetAllocateInfo));
    dsai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    dsai.descriptorPool = descriptorPool;
    dsai.descriptorSetCount = 1;
    dsai.pSetLayouts = &descriptorSetLayout;

    descriptorSet = {};

    auto allocResult = vkAllocateDescriptorSets(vkz_logical_device, &dsai,
      &descriptorSet);
    if (allocResult != VK_SUCCESS) {
      std::string errortxt = "Failed to allocate descriptor sets.";
      throw std::runtime_error(errortxt);
    }

    memset(dslb, 0, 5 * sizeof(VkDescriptorSetLayoutBinding));

    // textureShader - textureImage
    dslb[0].binding = 3;
    dslb[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    dslb[0].descriptorCount = 1;
    dslb[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dslb[0].pImmutableSamplers = NULL;

    memset(&dslci, 0, sizeof(VkDescriptorSetLayoutCreateInfo));
    dslci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    dslci.bindingCount = 1;
    dslci.pBindings = dslb;


    if (vkCreateDescriptorSetLayout(vkz_logical_device, &dslci, NULL,
      &textureDescriptorSetLayout) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create texture "
        "descriptor set layout.");
    }
    else {
      LOGDEBUG("Created texture descriptor set layout.");
    }

  }

  void Renderer::updateDescriptorSets() {

    VkDescriptorBufferInfo dbiWorld = {};
    dbiWorld.buffer = worldDetailsBuffers[currentSwapchainImageIndex];
    dbiWorld.offset = 0;
    dbiWorld.range = (16 + 4) * sizeof(float);

    VkDescriptorBufferInfo dbiOrientation = {};

    dbiOrientation.buffer =
      renderOrientationBuffersDynamic[currentSwapchainImageIndex];
    dbiOrientation.offset = 0;
    dbiOrientation.range = sizeof(UboOrientation);

    VkDescriptorBufferInfo dbiCamera = {};

    dbiCamera.buffer = cameraOrientationBuffers[currentSwapchainImageIndex];
    dbiCamera.offset = 0;
    dbiCamera.range = (3 * 16 + 4) * sizeof(float);

    VkDescriptorBufferInfo dbiColour = {};

    dbiColour.buffer = colourBuffersDynamic[currentSwapchainImageIndex];
    dbiColour.offset = 0;
    dbiColour.range = 4 * sizeof(float);

    VkDescriptorBufferInfo dbiLight = {};

    dbiLight.buffer = lightIntensityBuffers[currentSwapchainImageIndex];
    dbiLight.offset = 0;
    dbiLight.range = sizeof(float);

    std::vector<VkWriteDescriptorSet> wds(6);

    wds[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    wds[0].dstSet = descriptorSet;
    wds[0].dstBinding = 0;
    wds[0].dstArrayElement = 0;
    wds[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    wds[0].descriptorCount = 1;
    wds[0].pBufferInfo = &dbiWorld;
    wds[0].pImageInfo = NULL;
    wds[0].pTexelBufferView = NULL;

    wds[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    wds[1].dstSet = descriptorSet;
    wds[1].dstBinding = 1;
    wds[1].dstArrayElement = 0;
    wds[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    wds[1].descriptorCount = 1;
    wds[1].pBufferInfo = &dbiOrientation;
    wds[1].pImageInfo = NULL;
    wds[1].pTexelBufferView = NULL;

    wds[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    wds[2].dstSet = descriptorSet;
    wds[2].dstBinding = 2;
    wds[2].dstArrayElement = 0;
    wds[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    wds[2].descriptorCount = 1;
    wds[2].pBufferInfo = &dbiCamera;
    wds[2].pImageInfo = NULL;
    wds[2].pTexelBufferView = NULL;

    wds[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    wds[3].dstSet = descriptorSet;
    wds[3].dstBinding = 4;
    wds[3].dstArrayElement = 0;
    wds[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    wds[3].descriptorCount = 1;
    wds[3].pBufferInfo = &dbiColour;
    wds[3].pImageInfo = NULL;
    wds[3].pTexelBufferView = NULL;

    wds[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    wds[4].dstSet = descriptorSet;
    wds[4].dstBinding = 5;
    wds[4].dstArrayElement = 0;
    wds[4].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    wds[4].descriptorCount = 1;
    wds[4].pBufferInfo = &dbiLight;
    wds[4].pImageInfo = NULL;
    wds[4].pTexelBufferView = NULL;

    vkUpdateDescriptorSets(vkz_logical_device, 5, &wds[0], 0, NULL);
  }

  void Renderer::allocateOrthoDescriptorSets() {

    VkDescriptorSetLayoutBinding dslb = {};

    // simpleFragmentShader - uboColour
    dslb.binding = 1;
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    dslb.descriptorCount = 1;
    dslb.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dslb.pImmutableSamplers = NULL;

    VkDescriptorSetLayoutCreateInfo dslci = {};

    dslci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    dslci.bindingCount = 1;
    dslci.pBindings = &dslb;

    if (vkCreateDescriptorSetLayout(vkz_logical_device, &dslci, NULL,
      &orthoDescriptorSetLayout) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create orthographic descriptor"
			       " set layout.");
    }
    else {
      LOGDEBUG("Created orthographic descriptor set layout.");
    }

    VkDescriptorSetAllocateInfo dsai = {};

    dsai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    dsai.descriptorPool = orthoDescriptorPool;
    dsai.descriptorSetCount = 1;
    dsai.pSetLayouts = &orthoDescriptorSetLayout;

    orthoDescriptorSet = {};
    VkResult allocResult = vkAllocateDescriptorSets(vkz_logical_device, &dsai,
      &orthoDescriptorSet);
    if (allocResult != VK_SUCCESS) {
      std::string errortxt = "Failed to allocate orthographic pool descriptor"
	" sets.";
      throw std::runtime_error(errortxt);
    }

    // simpleFragmentShader - textureImage
    dslb = {};
    dslb.binding = 0;
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    dslb.descriptorCount = 1;
    dslb.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dslb.pImmutableSamplers = NULL;

    dslci = {};
    dslci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    dslci.bindingCount = 1;
    dslci.pBindings = &dslb;

    if (vkCreateDescriptorSetLayout(vkz_logical_device, &dslci, NULL,
      &textureOrthoDescriptorSetLayout) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create orthographic texture"
			       " descriptor set layout.");
    }
    else {
      LOGDEBUG("Created orthographic texture descriptor set layout.");
    }
  }

  void Renderer::updateOrthoDescriptorSets() {

    VkDescriptorBufferInfo dbiColour = {};
    dbiColour.buffer = colourBuffersDynamic[currentSwapchainImageIndex];
    dbiColour.offset = 0;
    dbiColour.range = 4 * sizeof(float);

    VkWriteDescriptorSet wds = {};
    wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    wds.dstSet = orthoDescriptorSet;
    wds.dstBinding = 1;
    wds.dstArrayElement = 0;
    wds.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    wds.descriptorCount = 1;
    wds.pBufferInfo = &dbiColour;
    wds.pImageInfo = NULL;
    wds.pTexelBufferView = NULL;

    vkUpdateDescriptorSets(vkz_logical_device, 1, &wds, 0, NULL);

  }

  void Renderer::setColourBuffer(glm::vec4 colour, uint32_t memIndex) {

    if (memIndex > maxObjectsPerPass) {
      std::string maxIndex = intToStr(maxObjectsPerPass - 1);
      throw std::runtime_error("Object colour buffer max index (" +
        maxIndex + ") exceeded.");
    }

    uboColourDynamic[memIndex] = {};

    uboColourDynamic[memIndex].colour = colour;

  }

  void Renderer::positionNextObject(const glm::vec3 offset,
    const glm::vec3 rotation, uint32_t memIndex) {

    if (memIndex > maxObjectsPerPass) {
      std::string maxObjects = intToStr(maxObjectsPerPass);
      throw std::runtime_error("Cannot position more than " + maxObjects +
			       " on the scene.");
    }

    uboOrientationDynamic[memIndex] = {};

    uboOrientationDynamic[memIndex].xRotationMatrix =
      glm::transpose(glm::rotate(glm::mat4x4(1.0f), rotation.x,
      glm::vec3(1.0f, 0.0f, 0.0f)));
    uboOrientationDynamic[memIndex].yRotationMatrix =
      glm::transpose(glm::rotate(glm::mat4x4(1.0f), rotation.y,
      glm::vec3(0.0f, 1.0f, 0.0f)));
    uboOrientationDynamic[memIndex].zRotationMatrix =
      glm::transpose(glm::rotate(glm::mat4x4(1.0f), rotation.z,
      glm::vec3(0.0f, 0.0f, 1.0f)));
    uboOrientationDynamic[memIndex].offset = offset;

  }

  void Renderer::positionCamera() {

    UboCamera camera = {};

    camera.position = cameraPosition;
    camera.xRotationMatrix = glm::transpose(glm::rotate(glm::mat4x4(1.0f),
							cameraRotation.x,
      glm::vec3(-1.0f, 0.0f, 0.0f)));
    camera.yRotationMatrix = glm::transpose(glm::rotate(glm::mat4x4(1.0f),
							cameraRotation.y,
      glm::vec3(0.0f, -1.0f, 0.0f)));
    camera.zRotationMatrix = glm::transpose(glm::rotate(glm::mat4x4(1.0f),
							cameraRotation.z,
      glm::vec3(0.0f, 0.0f, -1.0f)));

    uint32_t cameraOrientationSize = (3 * 16 + 4) * sizeof(float);

    if (cameraOrientationBuffers.size() == 0) {
      cameraOrientationBuffers.resize(vkz_swapchain_image_count);
      cameraOrientationBufferMemories.resize(vkz_swapchain_image_count);

      for (size_t i = 0; i < vkz_swapchain_image_count; i++) {
        vkz_create_buffer(&cameraOrientationBuffers[i],
			  VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
          cameraOrientationSize,
          &cameraOrientationBufferMemories[i],
          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
      }

    }

    void* orientationData;
    vkMapMemory(vkz_logical_device,
		cameraOrientationBufferMemories[currentSwapchainImageIndex],
      0, cameraOrientationSize, 0, &orientationData);
    memcpy(orientationData, &camera, cameraOrientationSize);
    vkUnmapMemory(vkz_logical_device,
		  cameraOrientationBufferMemories[currentSwapchainImageIndex]);

  }

  VulkanImage Renderer::getTextureHandle(const std::string name) const {
    VulkanImage handle;
    auto nameTexturePair = textures.find(name);
    if (nameTexturePair != textures.end()) {
      handle = nameTexturePair->second;
    }
    else {
      throw std::runtime_error("Could not find texture " + name);
    }
    return handle;
  }

  VulkanImage Renderer::generateTexture(const std::string name,
					const float* data,
    const unsigned long width,
    const unsigned long height,
    const bool replace) {
    
    bool found = false;
    
    VulkanImage textureHandle = {};

    for (auto &nameTexturePair : textures) {
      if (nameTexturePair.first == name) {
        if (!replace) {
          throw std::runtime_error("Texture with name " + name + 
            " already exists and replace flag not set.");
        }
        textureHandle = nameTexturePair.second;
        found = true;
        break;
      }
    }

    if (found) {
      deleteTexture(name);
      textureHandle.image = VK_NULL_HANDLE;
      textureHandle.imageView = VK_NULL_HANDLE;
      textureHandle.imageMemory = VK_NULL_HANDLE;
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    uint32_t imageByteSize = static_cast<uint32_t>(width * height * 4 * sizeof(float));

    if (!vkz_create_buffer(&stagingBuffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      imageByteSize, &stagingBufferMemory, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
      throw std::runtime_error("Failed to create staging buffer for texture.");
    }

    void* imgData;
    vkMapMemory(vkz_logical_device, stagingBufferMemory, 0, VK_WHOLE_SIZE,
		0, &imgData);
    memcpy(imgData, data, imageByteSize);
    vkUnmapMemory(vkz_logical_device, stagingBufferMemory);

    if (!vkz_create_image(&textureHandle.image, static_cast<uint32_t>(width),
                          static_cast<uint32_t>(height),
      VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_TRANSFER_DST_BIT |
      VK_IMAGE_USAGE_SAMPLED_BIT,
      &textureHandle.imageMemory,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
      throw std::runtime_error("Failed to create image to be used"
			       " as a texture.");
    }

    vkz_transition_image_layout(textureHandle.image,
				VK_FORMAT_R32G32B32A32_SFLOAT,
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    vkz_copy_buffer_to_image(stagingBuffer, textureHandle.image,
      (uint32_t)width, (uint32_t)height);

    vkz_destroy_buffer(stagingBuffer, stagingBufferMemory);

    vkz_transition_image_layout(textureHandle.image,
				VK_FORMAT_R32G32B32A32_SFLOAT,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    if (!vkz_create_image_view(&textureHandle.imageView, textureHandle.image,
      VK_FORMAT_R32G32B32A32_SFLOAT,
      VK_IMAGE_ASPECT_COLOR_BIT)) {
      throw std::runtime_error("Failed to create texture image view!\n\r");
    };

    if (!found) {
    
      // Allocate perspective texture descriptor set

      VkDescriptorSetAllocateInfo dsai = {};

      dsai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
      dsai.descriptorPool = descriptorPool;
      dsai.descriptorSetCount = 1;
      dsai.pSetLayouts = &textureDescriptorSetLayout;

      textureHandle.descriptorSet = {};

      VkResult allocResult = vkAllocateDescriptorSets(vkz_logical_device, &dsai,
        &textureHandle.descriptorSet);
      if (allocResult != VK_SUCCESS) {
        std::string errortxt = "Failed to allocate texture descriptor set.";
        throw std::runtime_error(errortxt);
      }
    }

    // Write texture descriptor set

    VkDescriptorImageInfo diiTexture = {};

    diiTexture.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    diiTexture.imageView = textureHandle.imageView;
    diiTexture.sampler = textureSampler;

    VkWriteDescriptorSet wds = {};

    wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    wds.dstSet = textureHandle.descriptorSet;
    wds.dstBinding = 3;
    wds.dstArrayElement = 0;
    wds.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    wds.descriptorCount = 1;
    wds.pBufferInfo = NULL;
    wds.pImageInfo = &diiTexture;
    wds.pTexelBufferView = NULL;

    vkUpdateDescriptorSets(vkz_logical_device, 1, &wds, 0, NULL);

    if (!found) {
      
      // Allocate orthographic texture descriptor set
      
      VkDescriptorSetAllocateInfo dsai = {};

      dsai = {};
      dsai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
      dsai.descriptorPool = descriptorPool;
      dsai.descriptorSetCount = 1;
      dsai.pSetLayouts = &textureOrthoDescriptorSetLayout;

      textureHandle.orthoDescriptorSet = {};

      VkResult allocResult = vkAllocateDescriptorSets(vkz_logical_device, &dsai,
        &textureHandle.orthoDescriptorSet);
      if (allocResult != VK_SUCCESS) {
        std::string errortxt = "Failed to allocate orthographic"
          " texture descriptor set.";
        throw std::runtime_error(errortxt);
      }
    }

    // Write orthographic texture descriptor set

    diiTexture = {};
    diiTexture.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    diiTexture.imageView = textureHandle.imageView;
    diiTexture.sampler = textureSampler;

    wds = {};

    wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    wds.dstSet = textureHandle.orthoDescriptorSet;
    wds.dstBinding = 0;
    wds.dstArrayElement = 0;
    wds.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    wds.descriptorCount = 1;
    wds.pBufferInfo = NULL;
    wds.pImageInfo = &diiTexture;
    wds.pTexelBufferView = NULL;

    vkUpdateDescriptorSets(vkz_logical_device, 1, &wds, 0, NULL);

    textures.insert(make_pair(name, textureHandle));

    return textureHandle;
  }

  void Renderer::init(const int width, const int height,
    const std::string shadersPath) {

    realScreenWidth = width;
    realScreenHeight = height;

    this->shadersPath = shadersPath;
    
#ifdef SMALL3D_IOS
    std::string basePath = get_base_path();
    basePath += "/";
    this->shadersPath = basePath + this->shadersPath;
#endif

    this->initWindow(realScreenWidth, realScreenHeight);

    LOGDEBUG("Detected back width " + intToStr(realScreenWidth) +
    " height " + intToStr(realScreenHeight));

    this->initVulkan();

    std::string vertexShaderPath = this->shadersPath +
      "perspectiveMatrixLightedShader.spv";
    std::string fragmentShaderPath = this->shadersPath +
      "textureShader.spv";

    if (!vkz_create_sampler(&textureSampler)) {
      throw std::runtime_error("Failed to create the sampler!");
    }

    vkz_create_pipeline(vertexShaderPath.c_str(), fragmentShaderPath.c_str(),
      setInputStateCallback, setPipelineLayoutCallback,
			&perspectivePipelineIndex);

    boundTextureViews.resize(vkz_swapchain_image_count);

    Image blankImage("");
    blankImage.toColour(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
    generateTexture("blank", blankImage);
    for (int i = 0; i < boundTextureViews.size(); ++i) {
      boundTextureViews[i] = getTextureHandle("blank").imageView;
    }

    std::string orthoVertexShaderPath = this->shadersPath +
      "simpleVertexShader.spv";
    std::string orthoFragmentShaderPath = this->shadersPath +
      "simpleFragmentShader.spv";

    vkz_create_pipeline(orthoVertexShaderPath.c_str(),
			orthoFragmentShaderPath.c_str(),
      setOrthoInputStateCallback, setOrthoPipelineLayoutCallback,
			&orthographicPipelineIndex);

    vkz_create_sync_objects();

    // Allocate memory & vulkan dynamic buffer for object positioning
    VkPhysicalDeviceProperties pdp = {};
    vkGetPhysicalDeviceProperties(vkz_physical_device, &pdp);
    VkDeviceSize minAlignment = pdp.limits.minUniformBufferOffsetAlignment;

    dynamicOrientationAlignment = sizeof(UboOrientation);
    if (minAlignment > 0) {
      dynamicOrientationAlignment = (dynamicOrientationAlignment +
				     minAlignment - 1) & ~(minAlignment - 1);
    }

    uboOrientationDynamicSize = maxObjectsPerPass * dynamicOrientationAlignment;
    char* mem = alloc.allocate(uboOrientationDynamicSize);
    std::fill(mem, &mem[uboOrientationDynamicSize], 0);
    uboOrientationDynamic = (UboOrientation*)mem;

    renderOrientationBuffersDynamic.resize(vkz_swapchain_image_count);
    renderOrientationBuffersDynamicMemory.resize(vkz_swapchain_image_count);

    for (size_t i = 0; i < vkz_swapchain_image_count; ++i) {
      vkz_create_buffer(&renderOrientationBuffersDynamic[i],
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        (uint32_t)uboOrientationDynamicSize,
        &renderOrientationBuffersDynamicMemory[i],
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }

    // Allocate memory & vulkan dynamic buffer for colour
    dynamicColourAlignment = sizeof(UboColour);
    if (minAlignment > 0) {
      dynamicColourAlignment = (dynamicColourAlignment + minAlignment - 1) &
	~(minAlignment - 1);
    }

    uboColourDynamicSize = maxObjectsPerPass * dynamicColourAlignment;
    mem = alloc.allocate(uboColourDynamicSize);
    std::fill(mem, &mem[uboColourDynamicSize], 0);
    uboColourDynamic = (UboColour*)mem;


    colourBuffersDynamic.resize(vkz_swapchain_image_count);
    colourBuffersDynamicMemory.resize(vkz_swapchain_image_count);


    for (size_t i = 0; i < vkz_swapchain_image_count; ++i) {
      vkz_create_buffer(&colourBuffersDynamic[i],
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        (uint32_t)uboColourDynamicSize,
        &colourBuffersDynamicMemory[i],
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }
    // end of memory allocation for object positioning

  }

  void Renderer::initWindow(int& width, int& height) {
#if defined(__ANDROID__)
    assert(vkz_android_app->window != nullptr);
    width = ANativeWindow_getWidth(vkz_android_app->window);
    height = ANativeWindow_getHeight(vkz_android_app->window);
#elif defined(SMALL3D_IOS)
    width = get_app_width();
    height = get_app_height();
#else
    glfwSetErrorCallback(errorCallback);

    if (!glfwInit()) {
      throw std::runtime_error("Unable to initialise GLFW");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    GLFWmonitor* monitor = nullptr; // If NOT null, a full-screen window will
    // be created.

    if ((width == 0 && height != 0) || (width != 0 && height == 0)) {
      throw std::runtime_error("Screen width and height both have to be equal "
        "or not equal to zero at the same time.");
    }
    else if (width == 0) {

      monitor = glfwGetPrimaryMonitor();

      const GLFWvidmode* mode = glfwGetVideoMode(monitor);
      // Full screen on a mac is very slow, hence this hack
#if defined(__APPLE__) && !defined(SMALL3D_IOS)
      width = mode->width * 0.8;
      height = mode->height * 0.8;
      monitor = nullptr; // back to windowed mode
#else
      width = mode->width;
      height = mode->height;
#endif
      LOGINFO("Detected screen width " + intToStr(width) + " and height " +
	      intToStr(height));
    }

    window = glfwCreateWindow(width, height, windowTitle.c_str(), monitor,
      nullptr);
    if (!window) {
      throw std::runtime_error("Unable to create GLFW window");
    }

    width = 0;
    height = 0;
    glfwGetFramebufferSize(window, &width, &height);

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    LOGINFO("Framebuffer width " + intToStr(width) + " height " +
	    intToStr(height));
    
#endif
  }

  void Renderer::setPerspectiveAndLight() {

    UboWorld world = {};

    float tmpmat4[16];
    memset(&tmpmat4, 0, 16 * sizeof(float));
    tmpmat4[0] = frustumScale;
    tmpmat4[5] = frustumScale * realScreenWidth / realScreenHeight;
    tmpmat4[10] = (zNear + zFar) / (zNear - zFar);
    tmpmat4[11] = 2.0f * zNear * zFar / (zNear - zFar);
    tmpmat4[14] = zOffsetFromCamera;
    world.perspectiveMatrix = glm::make_mat4(tmpmat4);
    world.lightDirection = lightDirection;

    uint32_t worldDetailsSize = (16 + 4) * sizeof(float);

    if (worldDetailsBuffers.size() == 0) {
      worldDetailsBuffers = std::vector<VkBuffer>(vkz_swapchain_image_count);
      worldDetailsBufferMemories =
	std::vector<VkDeviceMemory>(vkz_swapchain_image_count);

      for (size_t i = 0; i < vkz_swapchain_image_count; i++) {
        vkz_create_buffer(&worldDetailsBuffers[i],
			  VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
          worldDetailsSize,
          &worldDetailsBufferMemories[i],
          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
      }
    }

    void* worldDetailsData;
    vkMapMemory(vkz_logical_device,
		worldDetailsBufferMemories[currentSwapchainImageIndex],
      0, worldDetailsSize, 0, &worldDetailsData);
    memcpy(worldDetailsData, &world, worldDetailsSize);
    vkUnmapMemory(vkz_logical_device,
		  worldDetailsBufferMemories[currentSwapchainImageIndex]);

    UboLight light = {};

    light.intensity = lightIntensity;

    uint32_t lightIntensitySize = sizeof(float);

    if (lightIntensityBuffers.size() == 0) {
      lightIntensityBuffers = std::vector<VkBuffer>(vkz_swapchain_image_count);
      lightIntensityBufferMemories =
	std::vector<VkDeviceMemory>(vkz_swapchain_image_count);

      for (size_t i = 0; i < vkz_swapchain_image_count; i++) {
        vkz_create_buffer(&lightIntensityBuffers[i],
			  VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
          lightIntensitySize,
          &lightIntensityBufferMemories[i],
          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
      }
    }

    void* lightIntensityData;
    vkMapMemory(vkz_logical_device,
		lightIntensityBufferMemories[currentSwapchainImageIndex],
      0, lightIntensitySize, 0, &lightIntensityData);
    memcpy(lightIntensityData, &light, lightIntensitySize);
    vkUnmapMemory(vkz_logical_device,
		  lightIntensityBufferMemories[currentSwapchainImageIndex]);

  }

  Renderer::Renderer() {
#if !defined(__ANDROID__) && !defined(SMALL3D_IOS)
    window = 0;
#endif
    lightDirection = glm::vec3(0.0f, 0.9f, 0.2f);
    cameraPosition = glm::vec3(0, 0, 0);
    cameraRotation = glm::vec3(0, 0, 0);

  }

  Renderer::Renderer(const std::string &windowTitle, const int width,
    const int height, const float frustumScale,
    const float zNear, const float zFar,
    const float zOffsetFromCamera,
    const std::string &shadersPath,
    const uint32_t maxObjectsPerPass) {

#if !defined(__ANDROID__) && !defined(SMALL3D_IOS)
    window = 0;
#endif

    lightDirection = glm::vec3(0.0f, 0.9f, 0.2f);
    cameraPosition = glm::vec3(0, 0, 0);
    cameraRotation = glm::vec3(0, 0, 0);
    this->maxObjectsPerPass = maxObjectsPerPass;
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
  }

  int Renderer::getScreenWidth() {
    return realScreenWidth;
  }
   
  int Renderer::getScreenHeight(){
    return realScreenHeight;
  }

  Renderer& Renderer::getInstance(const std::string &windowTitle,
				  const int width, const int height,
				  const float frustumScale,
				  const float zNear, const float zFar,
				  const float zOffsetFromCamera,
				  const std::string &shadersPath,
				  const uint32_t maxObjectsPerPass) {

    static Renderer instance(windowTitle, width, height, frustumScale, zNear,
			     zFar, zOffsetFromCamera, shadersPath,
			     maxObjectsPerPass);
    return instance;
  }

  Renderer::~Renderer() {
    
    LOGDEBUG("Renderer destructor running");

    vkDeviceWaitIdle(vkz_logical_device);
    
    for (auto model : garbageModels) {
      clearBuffers(model);
    }
    garbageModels.clear();
    
    for (auto it = textures.begin();
      it != textures.end(); ++it) {
      LOGDEBUG("Deleting texture " + it->first);

      vkDestroyImageView(vkz_logical_device, it->second.imageView, NULL);
      vkz_destroy_image(it->second.image, it->second.imageMemory);

    }

    for (auto idFacePair : fontFaces) {
      FT_Done_Face(idFacePair.second);
    }

#ifdef __ANDROID__
    for (auto asset : fontAssets) {
      AAsset_close(asset);
    }
#endif

    FT_Done_FreeType(library);
    
    vkz_destroy_sync_objects();
    
    vkDestroyDescriptorSetLayout(vkz_logical_device,
      descriptorSetLayout, NULL);

    vkDestroyDescriptorSetLayout(vkz_logical_device,
      textureDescriptorSetLayout, NULL);

    vkDestroyDescriptorSetLayout(vkz_logical_device,
      orthoDescriptorSetLayout, NULL);

    vkDestroyDescriptorSetLayout(vkz_logical_device,
      textureOrthoDescriptorSetLayout, NULL);

    if (descriptorPoolCreated) {
      vkDestroyDescriptorPool(vkz_logical_device, descriptorPool, NULL);
    }

    if (orthoDescriptorPoolCreated) {
      vkDestroyDescriptorPool(vkz_logical_device, orthoDescriptorPool, NULL);
    }

    if (uboOrientationDynamic) {
      alloc.deallocate(reinterpret_cast<char*>(uboOrientationDynamic),
		       uboOrientationDynamicSize);
    }

    if (uboColourDynamic) {
      alloc.deallocate(reinterpret_cast<char*>(uboColourDynamic),
		       uboColourDynamicSize);
    }

    for (uint32_t i = 0; i < vkz_swapchain_image_count; ++i) {
      
      if (i < renderOrientationBuffersDynamic.size()) {
        vkz_destroy_buffer(renderOrientationBuffersDynamic[i],
          renderOrientationBuffersDynamicMemory[i]);
      }
      if (i < cameraOrientationBuffers.size()) {
        vkz_destroy_buffer(cameraOrientationBuffers[i],
          cameraOrientationBufferMemories[i]);
      }
      if (i < worldDetailsBuffers.size()) {
        vkz_destroy_buffer(worldDetailsBuffers[i],
          worldDetailsBufferMemories[i]);
      }
      if (i < lightIntensityBuffers.size()) {
        vkz_destroy_buffer(lightIntensityBuffers[i],
          lightIntensityBufferMemories[i]);
      }
      if (i < colourBuffersDynamic.size()) {
        vkz_destroy_buffer(colourBuffersDynamic[i],
          colourBuffersDynamicMemory[i]);
      }
    }

    vkDestroySampler(vkz_logical_device, textureSampler, NULL);
  
    vkz_destroy_swapchain();

    LOGDEBUG("Destroying surface.\n\r");
    
    vkDestroySurfaceKHR(vkz_instance, vkz_surface, NULL);

    vkz_shutdown();

    // The following causes an EXC_BAD_ACCESS,
    // or a "Thread 1: signal SIABRT" exception on MacOS. In
    // the latter case the following message is produced:
    // objc[986]: Attempt to use unknown class 0x1008b4810.
    
    // glfwDestroyWindow(window);
    
    // On linux this causes a segmentation fault
    // glfwTerminate();
  }

#if !defined(__ANDROID__) && !defined(SMALL3D_IOS)
  GLFWwindow* Renderer::getWindow() const {
    return window;
  }
#endif

  void Renderer::generateTexture(const std::string &name, const Image &image) {
    this->generateTexture(name, image.getData(), image.getWidth(),
      image.getHeight(), false);
  }

  void Renderer::generateTexture(const std::string &name, const std::string &text,
    const glm::vec3 &colour, const int fontSize,
    const std::string &fontPath, const bool replace) {

    std::string faceId = intToStr(fontSize) + fontPath;

    auto idFacePair = fontFaces.find(faceId);
    FT_Face face;
    FT_Error error;

    if (idFacePair == fontFaces.end()) {
      std::string faceFullPath = fontPath;
#ifdef SMALL3D_IOS
      std::string basePath = get_base_path();
      basePath += "/";
      faceFullPath = basePath + faceFullPath;
#endif
      
      LOGDEBUG("Loading font from " + faceFullPath);
#ifdef __ANDROID__
      AAsset *asset = AAssetManager_open(vkz_android_app->activity->assetManager,
                                         faceFullPath.c_str(),
                                         AASSET_MODE_STREAMING);
      if (!asset) throw std::runtime_error("Opening asset " + faceFullPath +
                                           " has failed!");
      off_t length;
      length = AAsset_getLength(asset);
      const void* buffer = AAsset_getBuffer(asset);
      error = FT_New_Memory_Face(library, (const FT_Byte*) buffer,
        length, 0, &face);
      fontAssets.push_back(asset);
#else
      error = FT_New_Face(library, faceFullPath.c_str(), 0, &face);
#endif
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

    textMemory.resize(4 * static_cast<size_t>(width) *
		      static_cast<size_t>(height) * sizeof(float));
    memset(&textMemory[0], 0, 4 * static_cast<size_t>(width) *
	   static_cast<size_t>(height) * sizeof(float));

    unsigned long totalAdvance = 0;

    for (const char& c : text) {
      error = FT_Load_Char(face, (FT_ULong)c, FT_LOAD_RENDER);
      if (error != 0) {
        throw std::runtime_error("Failed to load character glyph.");
      }

      FT_GlyphSlot slot = face->glyph;

      if (slot->bitmap.width * slot->bitmap.rows > 0) {
        for (size_t row = 0; row < static_cast<int>(slot->bitmap.rows); ++row) {
          for (size_t col = 0; col < static_cast<int>(slot->bitmap.width);
	       ++col) {

            glm::vec4 colourAlpha = glm::vec4(colour, 0.0f);

            colourAlpha.a =
              floorf(100.0f * (static_cast<float>
              (slot->bitmap.buffer[row * slot->bitmap.width +
                col]) /
                255.0f) + 0.5f) / 100.0f;

            size_t dst = 4 * (maxTop - static_cast<size_t>(slot->bitmap_top) +
			      row) *
              width + 4 * (static_cast<size_t>(slot->bitmap_left) + col)
              + totalAdvance;

            std::memcpy(&textMemory[dst], glm::value_ptr(colourAlpha),
              4 * sizeof(float));
          }
        }
      }
      totalAdvance += 4 * static_cast<unsigned long>(slot->advance.x / 64);
    }
    generateTexture(name, &textMemory[0], static_cast<unsigned long>(width),
      static_cast<unsigned long>(height), replace);
  }

  void Renderer::deleteTexture(const std::string &name) {
    auto nameTexturePair = textures.find(name);

    if (nameTexturePair != textures.end()) {

      vkDestroyImageView(vkz_logical_device,
			 nameTexturePair->second.imageView, NULL);
      vkz_destroy_image(nameTexturePair->second.image,
			nameTexturePair->second.imageMemory);

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

  void Renderer::render(Model & model, const glm::vec3 &offset,
    const glm::vec3 &rotation,
    const glm::vec4 &colour,
    const std::string &textureName,
    const bool perspective) {

#if defined(DEBUG) || defined(_DEBUG) || !defined (NDEBUG)
    if (model.indexData.size() == 0 ||
      model.indexDataByteSize == 0 ||
      model.vertexData.size() == 0 ||
      model.vertexDataByteSize == 0 ||
      model.normalsData.size() == 0 ||
      model.normalsDataByteSize == 0 ||
      model.textureCoordsData.size() == 0 ||
      model.textureCoordsDataByteSize == 0) {
      throw std::runtime_error("Model to be rendered has some empty values!");
    }
#endif

    if (!model.alreadyInGPU) {

      // Send vertex data to GPU

      if (!vkz_create_buffer(&model.positionBuffer,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        model.vertexDataByteSize,
        &model.positionBufferMemory,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
        throw std::runtime_error("Failed to create vertex buffer.");
      }

      VkBuffer stagingBuffer;
      VkDeviceMemory stagingBufferMemory;

      if (vkz_create_buffer(&stagingBuffer,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        model.vertexDataByteSize,
        &stagingBufferMemory,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
        void* stagingData;
        vkMapMemory(vkz_logical_device, stagingBufferMemory, 0,
          VK_WHOLE_SIZE,
          0, &stagingData);
        memcpy(stagingData, &model.vertexData[0], model.vertexDataByteSize);
        vkUnmapMemory(vkz_logical_device, stagingBufferMemory);

        vkz_copy_buffer(stagingBuffer, model.positionBuffer,
          model.vertexDataByteSize);

        vkz_destroy_buffer(stagingBuffer, stagingBufferMemory);
      }
      else {
        throw std::runtime_error("Failed to create the staging buffer"
				 " for vertices.");
      }

      // Send index data

      if (!vkz_create_buffer(&model.indexBuffer,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        model.indexDataByteSize,
        &model.indexBufferMemory,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
        throw std::runtime_error("Failed to create index buffer.");
      }

      if (vkz_create_buffer(&stagingBuffer,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        model.indexDataByteSize,
        &stagingBufferMemory,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
        void* stagingData;
        vkMapMemory(vkz_logical_device, stagingBufferMemory, 0,
          VK_WHOLE_SIZE,
          0, &stagingData);
        memcpy(stagingData, &model.indexData[0], model.indexDataByteSize);
        vkUnmapMemory(vkz_logical_device, stagingBufferMemory);

        vkz_copy_buffer(stagingBuffer, model.indexBuffer,
          model.indexDataByteSize);

        vkz_destroy_buffer(stagingBuffer, stagingBufferMemory);
      }
      else {
        throw std::runtime_error("Failed to create the staging"
				 " buffer for indices.");
      }

      // Send normals data

      if (!vkz_create_buffer(&model.normalsBuffer,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        model.normalsDataByteSize,
        &model.normalsBufferMemory,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
        throw std::runtime_error("Failed to create normals buffer.");
      }

      if (vkz_create_buffer(&stagingBuffer,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        model.normalsDataByteSize,
        &stagingBufferMemory,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
        void* normalsData;
        vkMapMemory(vkz_logical_device, stagingBufferMemory, 0,
          VK_WHOLE_SIZE,
          0, &normalsData);
        memcpy(normalsData, &model.normalsData[0], model.normalsDataByteSize);
        vkUnmapMemory(vkz_logical_device, stagingBufferMemory);

        vkz_copy_buffer(stagingBuffer, model.normalsBuffer,
          model.normalsDataByteSize);

        vkz_destroy_buffer(stagingBuffer, stagingBufferMemory);
      }
      else {
        throw std::runtime_error("Failed to create the staging"
				 " buffer for indices.");
      }

      if (model.textureCoordsDataByteSize != 0) {

        // Send texture coordinates data

        if (!vkz_create_buffer(&model.uvBuffer,
          VK_BUFFER_USAGE_TRANSFER_DST_BIT |
          VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
          model.textureCoordsDataByteSize,
          &model.uvBufferMemory,
          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
          throw std::runtime_error("Failed to create uv buffer.");
        }

        if (vkz_create_buffer(&stagingBuffer,
          VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
          model.textureCoordsDataByteSize,
          &stagingBufferMemory,
          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
          void* uvData;
          vkMapMemory(vkz_logical_device, stagingBufferMemory, 0,
            VK_WHOLE_SIZE,
            0, &uvData);
          memcpy(uvData, &model.textureCoordsData[0],
		 model.textureCoordsDataByteSize);
          vkUnmapMemory(vkz_logical_device, stagingBufferMemory);

          vkz_copy_buffer(stagingBuffer, model.uvBuffer,
            model.textureCoordsDataByteSize);

          vkz_destroy_buffer(stagingBuffer, stagingBufferMemory);
        }
        else {
          throw std::runtime_error("Failed to create the staging"
				   " buffer for texture coordinates.");
        }

      }

      model.alreadyInGPU = true;
    }

    if (textureName != "") {
      // "Disable" colour since there is a texture
      setColourBuffer(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), colourMemIndex);
      model.textureName = textureName;
    }
    else {
      
#if defined(__APPLE__) || defined(__ANDROID__)
      // On MacOS, and some Android devices, setting the model colour
      // corrupts the game's frames (some objects don't appear). This
      // workaround converts colours to textures.
      std::string texName = "col" + floatToStr(colour.r) +
	floatToStr(colour.g) +
        floatToStr(colour.b) + floatToStr(colour.a);
      auto nameTexPair = textures.find(texName);
      if (nameTexPair == textures.end()) {
        Image t;
        t.toColour(colour);
        generateTexture(texName, t);
      }
      model.textureName = texName;
      setColourBuffer(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), colourMemIndex);
#else
      // If there is no texture, use the given colour
      setColourBuffer(colour, colourMemIndex);
      model.textureName = "blank";
#endif
    }

    model.colourMemIndex = colourMemIndex;
    ++colourMemIndex;

    model.perspective = perspective;

    if (perspective) {
      positionNextObject(offset, rotation, orientationMemIndex);
      model.orientationMemIndex = orientationMemIndex;
      ++orientationMemIndex;
    }

    nextModelsToDraw.push_back(model);

  }

  void Renderer::render(Model & model, const glm::vec3 &offset,
    const glm::vec3 &rotation,
    const std::string &textureName) {
    this->render(model, offset, rotation, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
      textureName);
  }

  void Renderer::render(Model& model, const std::string& textureName,
    const bool perspective) {
    this->render(model, glm::vec3(0.0f, 0.0f,0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
      textureName, perspective);
  }

  void Renderer::render(Model& model, const glm::vec4 &colour,
    const bool perspective) {
    this->render(model, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), colour,
      "", perspective);
  }

  void Renderer::render(SceneObject &sceneObject,
    const glm::vec4 &colour) {
    this->render(sceneObject.getModel(), sceneObject.offset,
      sceneObject.rotation, colour, "");
  }

  void Renderer::render(SceneObject &sceneObject,
    const std::string &textureName) {
    this->render(sceneObject.getModel(), sceneObject.offset,
      sceneObject.rotation, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
      textureName);
  }

  void Renderer::clearBuffers(Model & model) const {
    if (model.alreadyInGPU) {
      vkz_destroy_buffer(model.positionBuffer, model.positionBufferMemory);
      vkz_destroy_buffer(model.indexBuffer, model.indexBufferMemory);
      vkz_destroy_buffer(model.normalsBuffer, model.normalsBufferMemory);
      vkz_destroy_buffer(model.uvBuffer, model.uvBufferMemory);
      model.alreadyInGPU = false;
    }
  }

  void Renderer::clearBuffers(SceneObject & sceneObject) const {
    for (Model &model : sceneObject.models) {
      clearBuffers(model);
    }
  }

  void Renderer::clearScreen() const {

    // Do nothing. Clearing is performed before rendering
    // anyway. This stub is left here for legacy compatibility
    // reasons.

  }

  void Renderer::clearScreen(const glm::vec4 &colour) {

    // Clearing is performed before rendering (see above).
    // Here only the clear colour is set.

    vkz_clear_colour.float32[0] = colour.r;
    vkz_clear_colour.float32[1] = colour.g;
    vkz_clear_colour.float32[2] = colour.b;
    vkz_clear_colour.float32[3] = colour.a;
  }

  void Renderer::swapBuffers() {

    vkz_acquire_next_image(perspectivePipelineIndex,
			   &currentSwapchainImageIndex);

    orientationMemIndex = 0;
    colourMemIndex = 0;

    setPerspectiveAndLight();
    positionCamera();

    // Updating object positioning 
    void* orientationData;
    vkMapMemory(vkz_logical_device,
	   renderOrientationBuffersDynamicMemory[currentSwapchainImageIndex],
      0, uboOrientationDynamicSize, 0, &orientationData);
    memcpy(orientationData, uboOrientationDynamic, uboOrientationDynamicSize);
    vkUnmapMemory(vkz_logical_device,
	   renderOrientationBuffersDynamicMemory[currentSwapchainImageIndex]);

    // Updating colour
    void* colourData;
    vkMapMemory(vkz_logical_device,
		colourBuffersDynamicMemory[currentSwapchainImageIndex],
      0, uboColourDynamicSize, 0, &colourData);
    memcpy(colourData, uboColourDynamic, uboColourDynamicSize);
    vkUnmapMemory(vkz_logical_device,
		  colourBuffersDynamicMemory[currentSwapchainImageIndex]);

    // All of the above is bound here.
    updateDescriptorSets();
    updateOrthoDescriptorSets();

    vkz_begin_draw_command_buffer(&nextCommandBuffer);

    for (auto model : nextModelsToDraw) {
      if (model.perspective) {
        
        vkz_bind_pipeline_to_command_buffer(perspectivePipelineIndex,
					    &nextCommandBuffer);
        bindBuffers(nextCommandBuffer, model);
        recordDrawCommand(nextCommandBuffer,
			  vkz_pipeline_layout[perspectivePipelineIndex],
          model, currentSwapchainImageIndex);
      }
      else {
        vkz_bind_pipeline_to_command_buffer(orthographicPipelineIndex,
					    &nextCommandBuffer);
        bindOrthoBuffers(nextCommandBuffer, model);
        recordOrthoDrawCommand(nextCommandBuffer,
			       vkz_pipeline_layout[orthographicPipelineIndex],
          model, currentSwapchainImageIndex);

      }
    }

    vkz_end_draw_command_buffer(&nextCommandBuffer);

    vkz_draw(&nextCommandBuffer);

    vkz_destroy_draw_command_buffer(&nextCommandBuffer);

    vkz_present_next_image();

    nextModelsToDraw.clear();

    for (auto model : garbageModels) {
      clearBuffers(model);
    }
    garbageModels.clear();

  }

}
