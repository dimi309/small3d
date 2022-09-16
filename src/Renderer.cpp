/*
 * Renderer.cpp
 *
 *  Created on: 2014/10/19
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#ifdef SMALL3D_IOS
#define VULKAN_HELPER_IOS
#endif

extern "C" {
#include "vulkan_helper.h"
}
#include "Renderer.hpp"
#include <stdexcept>
#include <fstream>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include "BasePath.hpp"
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
   * @brief Light uniform buffer object. Used internally
   */
  struct UboLight {
    float lightIntensity;
    float padding[63];
  };

  std::vector<Model> Renderer::nextModelsToDraw;

  VkVertexInputBindingDescription Renderer::bd[5];
  VkVertexInputAttributeDescription Renderer::ad[5];
  VkDescriptorSetLayout Renderer::descriptorSetLayout;
  VkDescriptorSet Renderer::descriptorSet[MAX_FRAMES_PREPARED];
  VkDescriptorSetLayout Renderer::textureDescriptorSetLayout;
  VkDescriptorSetLayout Renderer::perspectiveLayouts[2];

  int Renderer::realScreenWidth;
  int Renderer::realScreenHeight;

#if !defined(__ANDROID__) && !defined(SMALL3D_IOS)
  void Renderer::framebufferSizeCallback(GLFWwindow* window, int width,
    int height) {
    realScreenWidth = width;
    realScreenHeight = height;
    if (realScreenWidth == 0) realScreenWidth = 1;
    if (realScreenHeight == 0) realScreenHeight = 1;
    vh_set_width_height(width, height);
    LOGDEBUG("New framebuffer dimensions " + std::to_string(realScreenWidth) + " x " +
      std::to_string(realScreenWidth));
    vh_recreate_pipelines_and_swapchain();
  }
#endif

  int Renderer::setInputStateCallback(VkPipelineVertexInputStateCreateInfo*
    inputStateCreateInfo) {

    memset(bd, 0, 5 * sizeof(VkVertexInputBindingDescription));

    bd[0].binding = 0;
    bd[0].stride = 4 * sizeof(float);

    bd[1].binding = 1;
    bd[1].stride = 3 * sizeof(float);

    bd[2].binding = 2;
    bd[2].stride = 2 * sizeof(float);

    bd[3].binding = 3;
    bd[3].stride = 4;

    bd[4].binding = 4;
    bd[4].stride = 4 * sizeof(float);

    memset(ad, 0, 5 * sizeof(VkVertexInputAttributeDescription));

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

    ad[3].binding = 3;
    ad[3].location = 3;
    ad[3].format = VK_FORMAT_R8G8B8A8_UINT;
    ad[3].offset = 0;

    ad[4].binding = 4;
    ad[4].location = 4;
    ad[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    ad[4].offset = 0;

    inputStateCreateInfo->vertexBindingDescriptionCount = 5;
    inputStateCreateInfo->vertexAttributeDescriptionCount = 5;
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

    VkBuffer vertexBuffers[5];
    vertexBuffers[0] = model.positionBuffer;
    vertexBuffers[1] = model.normalsBuffer;
    vertexBuffers[2] = model.uvBuffer;
    vertexBuffers[3] = model.jointBuffer;
    vertexBuffers[4] = model.weightBuffer;

    VkDeviceSize offsets[5];
    offsets[0] = 0;
    offsets[1] = 0;
    offsets[2] = 0;
    offsets[3] = 0;
    offsets[4] = 0;

    uint32_t bindingCount = 5;

    // Vertex buffer
    vkCmdBindVertexBuffers(commandBuffer, 0, bindingCount, vertexBuffers, offsets);

    // Index buffer
    vkCmdBindIndexBuffer(commandBuffer, model.indexBuffer,
      0, VK_INDEX_TYPE_UINT32);
    return 1;
  }

  void Renderer::recordDrawCommand(VkCommandBuffer commandBuffer,
    VkPipelineLayout pipelineLayout, const Model& model,
    uint32_t swapchainImageIndex, bool perspective) {

    uint32_t dynamicModelPlacementOffset = model.placementMemIndex *
      static_cast<uint32_t>(dynamicModelPlacementAlignment);

    uint32_t dynamicWorldDetailsOffset = perspective ? 0 : 1 *
      static_cast<uint32_t>(dynamicWorldDetailsAlignment);

    uint32_t dynamicColourOffset = model.colourMemIndex *
      static_cast<uint32_t>(dynamicColourAlignment);

    const uint32_t dynamicOffsets[3] = { dynamicWorldDetailsOffset,
      dynamicModelPlacementOffset,
      dynamicColourOffset };

    const VkDescriptorSet descriptorSets[2] =
    { descriptorSet[currentFrameIndex], getTextureHandle(model.textureName).descriptorSet[currentFrameIndex] };

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
      pipelineLayout, 0, 2,
      descriptorSets, 3, dynamicOffsets);

    vkCmdDrawIndexed(commandBuffer, (uint32_t)model.indexData.size(),
      1, 0, 0, 0);
  }

  void Renderer::createDescriptorPool() {

    VkDescriptorPoolSize ps[3];

    memset(ps, 0, 3 * sizeof(VkDescriptorPoolSize));

    ps[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    ps[0].descriptorCount = 3 * MAX_FRAMES_PREPARED; // For uboWorld, uboModelPlacement and uboColour (3) 

    ps[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    ps[1].descriptorCount = MAX_FRAMES_PREPARED * objectsPerFrame;

    ps[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ps[2].descriptorCount = MAX_FRAMES_PREPARED; // For uboLight

    VkDescriptorPoolCreateInfo dpci;
    memset(&dpci, 0, sizeof(VkDescriptorPoolCreateInfo));
    dpci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    dpci.poolSizeCount = 3;
    dpci.pPoolSizes = ps;
    dpci.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    dpci.maxSets = MAX_FRAMES_PREPARED * objectsPerFrame;
    // No point in VS C26812 unscoped enum warning on VkResult.
    // small3d does not define that code itself.
#pragma warning(push)
#pragma warning(disable:26812)
    if (vkCreateDescriptorPool(vh_logical_device, &dpci, NULL,
      &descriptorPool) != VK_SUCCESS) {
      LOGDEBUG("Failed to create descriptor pool.");
    }
    else {
      LOGDEBUG("Created descriptor pool.");
    }
#pragma warning(pop)
  }

  void Renderer::destroyDescriptorPool() {
    vkDestroyDescriptorPool(vh_logical_device, descriptorPool, NULL);
    descriptorPool = VK_NULL_HANDLE;
  }

  void Renderer::allocateDescriptorSets() {

    VkDescriptorSetLayoutBinding dslb[4];
    memset(dslb, 0, 4 * sizeof(VkDescriptorSetLayoutBinding));

    // perspectiveMatrixLightedShader - uboWorld
    dslb[0].binding = worldDescBinding;
    dslb[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    dslb[0].descriptorCount = 1;
    dslb[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    dslb[0].pImmutableSamplers = NULL;

    // perspectiveMatrixLightedShader - uboModelPlacement

    dslb[1].binding = modelPlacementDescBinding;
    dslb[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    dslb[1].descriptorCount = 1;
    dslb[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    dslb[1].pImmutableSamplers = NULL;

    // textureShader - uboColour
    dslb[2].binding = colourDescBinding;
    dslb[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    dslb[2].descriptorCount = 1;
    dslb[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dslb[2].pImmutableSamplers = NULL;

    // textureShader - uboLight
    dslb[3].binding = lightDescBinding;
    dslb[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dslb[3].descriptorCount = 1;
    dslb[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dslb[3].pImmutableSamplers = NULL;

    VkDescriptorSetLayoutCreateInfo dslci;
    memset(&dslci, 0, sizeof(VkDescriptorSetLayoutCreateInfo));
    dslci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    dslci.bindingCount = 4;
    dslci.pBindings = dslb;

    if (vkCreateDescriptorSetLayout(vh_logical_device, &dslci, NULL,
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

    for (uint32_t idx = 0; idx < MAX_FRAMES_PREPARED; ++idx) {
      descriptorSet[idx] = {};

      if (vkAllocateDescriptorSets(vh_logical_device, &dsai,
        &descriptorSet[idx]) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate descriptor sets.");
      }
    }

    memset(dslb, 0, 4 * sizeof(VkDescriptorSetLayoutBinding));

    // textureShader - textureImage
    dslb[0].binding = textureDescBinding;
    dslb[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    dslb[0].descriptorCount = 1;
    dslb[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dslb[0].pImmutableSamplers = NULL;

    memset(&dslci, 0, sizeof(VkDescriptorSetLayoutCreateInfo));
    dslci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    dslci.bindingCount = 1;
    dslci.pBindings = dslb;


    if (vkCreateDescriptorSetLayout(vh_logical_device, &dslci, NULL,
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
    dbiWorld.buffer = worldDetailsBuffersDynamic[currentFrameIndex];
    dbiWorld.offset = 0;
    dbiWorld.range = sizeof(UboWorldDetails);

    VkDescriptorBufferInfo dbiModelPlacement = {};

    dbiModelPlacement.buffer =
      renderModelPlacementBuffersDynamic[currentFrameIndex];
    dbiModelPlacement.offset = 0;
    dbiModelPlacement.range = sizeof(UboModelPlacement);

    VkDescriptorBufferInfo dbiColour = {};

    dbiColour.buffer = colourBuffersDynamic[currentFrameIndex];
    dbiColour.offset = 0;
    dbiColour.range = sizeof(UboColour);

    VkDescriptorBufferInfo dbiLight = {};

    dbiLight.buffer = lightIntensityBuffers[currentFrameIndex];
    dbiLight.offset = 0;
    dbiLight.range = sizeof(UboLight);

    std::vector<VkWriteDescriptorSet> wds(4);

    wds[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    wds[0].dstSet = descriptorSet[currentFrameIndex];
    wds[0].dstBinding = worldDescBinding;
    wds[0].dstArrayElement = 0;
    wds[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    wds[0].descriptorCount = 1;
    wds[0].pBufferInfo = &dbiWorld;
    wds[0].pImageInfo = NULL;
    wds[0].pTexelBufferView = NULL;

    wds[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    wds[1].dstSet = descriptorSet[currentFrameIndex];
    wds[1].dstBinding = modelPlacementDescBinding;
    wds[1].dstArrayElement = 0;
    wds[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    wds[1].descriptorCount = 1;
    wds[1].pBufferInfo = &dbiModelPlacement;
    wds[1].pImageInfo = NULL;
    wds[1].pTexelBufferView = NULL;

    wds[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    wds[2].dstSet = descriptorSet[currentFrameIndex];
    wds[2].dstBinding = colourDescBinding;
    wds[2].dstArrayElement = 0;
    wds[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    wds[2].descriptorCount = 1;
    wds[2].pBufferInfo = &dbiColour;
    wds[2].pImageInfo = NULL;
    wds[2].pTexelBufferView = NULL;

    wds[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    wds[3].dstSet = descriptorSet[currentFrameIndex];
    wds[3].dstBinding = lightDescBinding;
    wds[3].dstArrayElement = 0;
    wds[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    wds[3].descriptorCount = 1;
    wds[3].pBufferInfo = &dbiLight;
    wds[3].pImageInfo = NULL;
    wds[3].pTexelBufferView = NULL;

    vkUpdateDescriptorSets(vh_logical_device, 4, &wds[0], 0, NULL);
  }

  void Renderer::destroyDescriptorSets() {
    for (uint32_t idx = 0; idx < MAX_FRAMES_PREPARED; ++idx) {
      if (vkFreeDescriptorSets(vh_logical_device, descriptorPool, 1,
        &descriptorSet[idx]) != VK_SUCCESS) {
        throw std::runtime_error("Failed to free descriptor sets.");
      }
      descriptorSet[idx] = VK_NULL_HANDLE;
    }

    vkDestroyDescriptorSetLayout(vh_logical_device,
      descriptorSetLayout, NULL);

    vkDestroyDescriptorSetLayout(vh_logical_device,
      textureDescriptorSetLayout, NULL);
  }

  void Renderer::setColourBuffer(glm::vec4 colour, uint32_t memIndex) {

    if (memIndex >= objectsPerFrame) {
      LOGERROR("Max objects per pass number (" + std::to_string(objectsPerFrame) +
        ") exceeded.");
    }

    uboColourDynamic[memIndex] = {};

    uboColourDynamic[memIndex].modelColour = colour;

  }

  void Renderer::transform(Model& model, const glm::vec3 offset,
    const glm::mat4x4 rotation, uint32_t memIndex) {

    if (memIndex >= objectsPerFrame) {
      LOGERROR("Max objects per pass number (" + std::to_string(objectsPerFrame) +
        ") exceeded.");
    }

    uboModelPlacementDynamic[memIndex] = {};

    uboModelPlacementDynamic[memIndex].modelTransformation =
      rotation *
      glm::scale(glm::mat4x4(1.0f), model.scale) *
      glm::translate(glm::mat4x4(1.0f), model.origTranslation) *
      glm::toMat4(model.origRotation) *
      glm::scale(glm::mat4x4(1.0f), model.origScale) * model.origTransformation;

    uboModelPlacementDynamic[memIndex].modelOffset = offset;

    uboModelPlacementDynamic[memIndex].hasJoints = model.joints.size() > 0 ? 1U : 0U;
    uint64_t idx = 0;
    for (auto& joint : model.joints) {

      uboModelPlacementDynamic[memIndex].jointTransformations[idx] =

        glm::inverse(glm::translate(glm::mat4x4(1.0f), model.origTranslation) *
          glm::toMat4(model.origRotation) *
          glm::scale(glm::mat4x4(1.0f), model.origScale)) *

        model.getJointTransform(idx) *

        joint.inverseBindMatrix;

      ++idx;
    }

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

  void Renderer::generateTexture(const std::string name,
    const float* data,
    const unsigned long width,
    const unsigned long height,
    const bool replace) {

    bool found = false;

    VulkanImage textureHandleLocal;
    VulkanImage* textureHandlePtr;

    for (auto& nameTexturePair : textures) {
      if (nameTexturePair.first == name) {
        if (data && !replace) { // if found it must either be replaced or 
                                // just recopied to the GPU from previously
                                // given data (when data == null)
          throw std::runtime_error("Texture '" + name +
            "' already exists, data was given and the replace flag is not set.");
        }
        textureHandlePtr = &nameTexturePair.second;
        found = true;
        break;
      }
    }

    if (!found) {
      textureHandlePtr = &textureHandleLocal;
    }

    if (found && replace) {
      if (data && width && height) {
        textureHandlePtr = &textureHandleLocal;
        deleteTexture(name);
      }
      else {
        throw std::runtime_error("No data or no dimensions given to replace texture '" + name + "'.");
      }
    }

    uint32_t imageByteSize = 0;

    if ((!found && data) /*new texture*/ ||
      (found && replace) /*texture to be replaced*/) {
      textureHandlePtr->width = width;
      textureHandlePtr->height = height;
      textureHandlePtr->data->resize(static_cast<uint64_t>(width) * height * 4);
      imageByteSize = static_cast<uint32_t>(static_cast<uint64_t>(width) * height * 4 * sizeof(float));
      memcpy(&(*textureHandlePtr->data)[0], data, imageByteSize);
    } // Otherwise these values are kept and we are just recopying to the GPU
    else {
      imageByteSize = static_cast<uint32_t>(textureHandlePtr->width * textureHandlePtr->height * 4 * sizeof(float));
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    if (!vh_create_buffer(&stagingBuffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      imageByteSize, &stagingBufferMemory, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
      throw std::runtime_error("Failed to create staging buffer for texture.");
    }

    void* imgData;
    vkMapMemory(vh_logical_device, stagingBufferMemory, 0, VK_WHOLE_SIZE,
      0, &imgData);
    memcpy(imgData, &(*textureHandlePtr->data)[0], imageByteSize);
    vkUnmapMemory(vh_logical_device, stagingBufferMemory);

    if (!vh_create_image(&textureHandlePtr->image, static_cast<uint32_t>(textureHandlePtr->width),
      static_cast<uint32_t>(textureHandlePtr->height),
      VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_TRANSFER_DST_BIT |
      VK_IMAGE_USAGE_SAMPLED_BIT,
      &textureHandlePtr->imageMemory,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
      throw std::runtime_error("Failed to create image to be used"
        " as a texture.");
    }

    vh_transition_image_layout(textureHandlePtr->image,
      VK_FORMAT_R32G32B32A32_SFLOAT,
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    vh_copy_buffer_to_image(stagingBuffer, textureHandlePtr->image,
      (uint32_t)textureHandlePtr->width, (uint32_t)textureHandlePtr->height);

    vh_destroy_buffer(stagingBuffer, stagingBufferMemory);

    vh_transition_image_layout(textureHandlePtr->image,
      VK_FORMAT_R32G32B32A32_SFLOAT,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    if (!vh_create_image_view(&textureHandlePtr->imageView, textureHandlePtr->image,
      VK_FORMAT_R32G32B32A32_SFLOAT,
      VK_IMAGE_ASPECT_COLOR_BIT)) {
      throw std::runtime_error("Failed to create texture image view!\n\r");
    };

    // Allocate perspective texture descriptor set

    VkDescriptorSetAllocateInfo dsai = {};

    dsai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    dsai.descriptorPool = descriptorPool;
    dsai.descriptorSetCount = 1;
    dsai.pSetLayouts = &textureDescriptorSetLayout;

    for (int idx = 0; idx < MAX_FRAMES_PREPARED; ++idx) {
      textureHandlePtr->descriptorSet[idx] = {};
      if (vkAllocateDescriptorSets(vh_logical_device, &dsai,
        &textureHandlePtr->descriptorSet[idx]) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate texture descriptor set.");
      }

      // Write texture descriptor set

      VkDescriptorImageInfo diiTexture = {};

      diiTexture.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      diiTexture.imageView = textureHandlePtr->imageView;
      diiTexture.sampler = textureSampler;

      VkWriteDescriptorSet wds = {};

      wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      wds.dstSet = textureHandlePtr->descriptorSet[idx];
      wds.dstBinding = textureDescBinding;
      wds.dstArrayElement = 0;
      wds.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      wds.descriptorCount = 1;
      wds.pBufferInfo = NULL;
      wds.pImageInfo = &diiTexture;
      wds.pTexelBufferView = NULL;

      vkUpdateDescriptorSets(vh_logical_device, 1, &wds, 0, NULL);
    }



    if (!found || replace) {
      textures.insert(make_pair(name, *textureHandlePtr));
    }
  }

  void Renderer::init(const int width, const int height,
    const std::string shadersPath) {

    realScreenWidth = width;
    realScreenHeight = height;

    this->shadersPath = getBasePath() + shadersPath;

    this->initWindow(realScreenWidth, realScreenHeight);

    LOGDEBUG("Detected back width " + std::to_string(realScreenWidth) +
      " height " + std::to_string(realScreenHeight));

    setupVulkan();

    Image blankImage("");
    blankImage.toColour(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
    generateTexture("blank", blankImage);

  }

  void Renderer::increaseObjectsPerFrame(const uint32_t additionalObjects) {
    destroyVulkan();
    objectsPerFrame += additionalObjects;
    setupVulkan();
  }

  void Renderer::allocateDynamicBuffers() {
    VkPhysicalDeviceProperties pdp = {};
    vkGetPhysicalDeviceProperties(vh_physical_device, &pdp);
    VkDeviceSize minAlignment = pdp.limits.minUniformBufferOffsetAlignment;

    dynamicModelPlacementAlignment = sizeof(UboModelPlacement);
    if (minAlignment > 0) {
      dynamicModelPlacementAlignment = (dynamicModelPlacementAlignment +
        minAlignment - 1) & ~(minAlignment - 1);
    }

    uboModelPlacementDynamicSize = objectsPerFrame * dynamicModelPlacementAlignment;
    char* mem = alloc.allocate(uboModelPlacementDynamicSize);
    std::fill(mem, &mem[uboModelPlacementDynamicSize], 0);
    uboModelPlacementDynamic = (UboModelPlacement*)mem;

    renderModelPlacementBuffersDynamic.resize(MAX_FRAMES_PREPARED);
    renderModelPlacementBuffersDynamicMemory.resize(MAX_FRAMES_PREPARED);

    for (size_t i = 0; i < MAX_FRAMES_PREPARED; ++i) {
      vh_create_buffer(&renderModelPlacementBuffersDynamic[i],
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        (uint32_t)uboModelPlacementDynamicSize,
        &renderModelPlacementBuffersDynamicMemory[i],
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }

    // Allocate memory & vulkan dynamic buffers for world details
    dynamicWorldDetailsAlignment = sizeof(UboWorldDetails);
    if (minAlignment > 0) {
      dynamicWorldDetailsAlignment = (dynamicWorldDetailsAlignment +
        minAlignment - 1) & ~(minAlignment - 1);
    }

    // 2x, one for perspective, the other for orthographic rendering
    uboWorldDetailsDynamicSize = 2 * dynamicWorldDetailsAlignment;
    mem = alloc.allocate(uboWorldDetailsDynamicSize);
    std::fill(mem, &mem[uboWorldDetailsDynamicSize], 0);
    uboWorldDetailsDynamic = (UboWorldDetails*)mem;
    worldDetailsBuffersDynamic.resize(MAX_FRAMES_PREPARED);
    worldDetailsBuffersDynamicMemory.resize(MAX_FRAMES_PREPARED);

    for (size_t i = 0; i < MAX_FRAMES_PREPARED; ++i) {
      vh_create_buffer(&worldDetailsBuffersDynamic[i],
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        (uint32_t)uboWorldDetailsDynamicSize,
        &worldDetailsBuffersDynamicMemory[i],
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }

    // Allocate memory & vulkan dynamic buffer for model colour
    dynamicColourAlignment = sizeof(UboColour);
    if (minAlignment > 0) {
      dynamicColourAlignment = (dynamicColourAlignment + minAlignment - 1) &
        ~(minAlignment - 1);
    }

    uboColourDynamicSize = objectsPerFrame * dynamicColourAlignment;
    mem = alloc.allocate(uboColourDynamicSize);
    std::fill(mem, &mem[uboColourDynamicSize], 0);
    uboColourDynamic = (UboColour*)mem;


    colourBuffersDynamic.resize(MAX_FRAMES_PREPARED);
    colourBuffersDynamicMemory.resize(MAX_FRAMES_PREPARED);


    for (size_t i = 0; i < MAX_FRAMES_PREPARED; ++i) {
      vh_create_buffer(&colourBuffersDynamic[i],
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        (uint32_t)uboColourDynamicSize,
        &colourBuffersDynamicMemory[i],
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }

  }

  void Renderer::destroyDynamicBuffers() {

    if (uboModelPlacementDynamic) {
      alloc.deallocate(reinterpret_cast<char*>(uboModelPlacementDynamic),
        uboModelPlacementDynamicSize);
      uboModelPlacementDynamic = nullptr;
    }

    if (uboWorldDetailsDynamic) {
      alloc.deallocate(reinterpret_cast<char*>(uboWorldDetailsDynamic),
        uboWorldDetailsDynamicSize);
      uboWorldDetailsDynamic = nullptr;
    }

    if (uboColourDynamic) {
      alloc.deallocate(reinterpret_cast<char*>(uboColourDynamic),
        uboColourDynamicSize);
      uboColourDynamic = nullptr;
    }

    for (uint32_t i = 0; i < MAX_FRAMES_PREPARED; ++i) {

      if (i < renderModelPlacementBuffersDynamic.size()) {
        vh_destroy_buffer(renderModelPlacementBuffersDynamic[i],
          renderModelPlacementBuffersDynamicMemory[i]);
      }

      if (i < worldDetailsBuffersDynamic.size()) {
        vh_destroy_buffer(worldDetailsBuffersDynamic[i],
          worldDetailsBuffersDynamicMemory[i]);
      }

      if (i < lightIntensityBuffers.size()) {
        vh_destroy_buffer(lightIntensityBuffers[i],
          lightIntensityBufferMemories[i]);
      }
      if (i < colourBuffersDynamic.size()) {
        vh_destroy_buffer(colourBuffersDynamic[i],
          colourBuffersDynamicMemory[i]);
      }
    }

    renderModelPlacementBuffersDynamic.clear();
    renderModelPlacementBuffersDynamicMemory.clear();

    worldDetailsBuffersDynamic.clear();
    worldDetailsBuffersDynamicMemory.clear();

    lightIntensityBuffers.clear();
    lightIntensityBufferMemories.clear();

    colourBuffersDynamic.clear();
    colourBuffersDynamicMemory.clear();
  }

  void Renderer::initWindow(int& width, int& height) {
#if defined(__ANDROID__)
    assert(small3d_android_app->window != nullptr);
    width = ANativeWindow_getWidth(small3d_android_app->window);
    height = ANativeWindow_getHeight(small3d_android_app->window);
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

    width = 0;
    height = 0;
    glfwGetFramebufferSize(window, &width, &height);

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    LOGINFO("Framebuffer width " + std::to_string(width) + " height " +
      std::to_string(height));

#endif
}

  void Renderer::setWorldDetails(bool perspective) {

    uint32_t worldDetailsIndex = perspective ? 0 : 1;

    uboWorldDetailsDynamic[worldDetailsIndex] = {};

    uboWorldDetailsDynamic[worldDetailsIndex].perspectiveMatrix = perspective && realScreenHeight != 0 ?
      glm::perspective(fieldOfView, static_cast<float>(realScreenWidth / realScreenHeight), zNear, zFar) :
      glm::mat4x4(1);

    uboWorldDetailsDynamic[worldDetailsIndex].lightDirection = perspective ?
      lightDirection : glm::vec3(0.0f, 0.0f, 0.0f);

    uboWorldDetailsDynamic[worldDetailsIndex].cameraOffset = perspective ?
      cameraPosition : glm::vec3(0.0f, 0.0f, 0.0f);

    uboWorldDetailsDynamic[worldDetailsIndex].cameraTransformation = perspective ?
      cameraTransformation :
      glm::mat4x4(1);

  }

  void Renderer::setLightIntensity() {
    UboLight light = {};

    light.lightIntensity = lightIntensity;

    uint32_t lightIntensitySize = sizeof(UboLight);

    if (lightIntensityBuffers.size() == 0) {
      lightIntensityBuffers = std::vector<VkBuffer>(MAX_FRAMES_PREPARED);
      lightIntensityBufferMemories =
        std::vector<VkDeviceMemory>(MAX_FRAMES_PREPARED);

      for (size_t i = 0; i < MAX_FRAMES_PREPARED; i++) {
        vh_create_buffer(&lightIntensityBuffers[i],
          VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
          lightIntensitySize,
          &lightIntensityBufferMemories[i],
          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
      }
    }

    void* lightIntensityData;
    vkMapMemory(vh_logical_device,
      lightIntensityBufferMemories[currentFrameIndex],
      0, lightIntensitySize, 0, &lightIntensityData);
    memcpy(lightIntensityData, &light, lightIntensitySize);
    vkUnmapMemory(vh_logical_device,
      lightIntensityBufferMemories[currentFrameIndex]);
  }

  void Renderer::deleteImageFromGPU(VulkanImage& gpuImage) {
    vkDeviceWaitIdle(vh_logical_device);
    for (int idx = 0; idx < MAX_FRAMES_PREPARED; ++idx) {
      if (vkFreeDescriptorSets(vh_logical_device, descriptorPool, 1,
        &gpuImage.descriptorSet[idx]) != VK_SUCCESS) {
        throw std::runtime_error("Failed to free texture descriptor set.");
      }
    }

    vkDestroyImageView(vh_logical_device,
      gpuImage.imageView, NULL);
    vh_destroy_image(gpuImage.image,
      gpuImage.imageMemory);
  }

  Renderer::Renderer() {
#if !defined(__ANDROID__) && !defined(SMALL3D_IOS)
    window = 0;
#endif

  }

  Renderer::Renderer(const std::string& windowTitle, const int width,
    const int height, const float fieldOfView,
    const float zNear, const float zFar,
    const std::string& shadersPath,
    const uint32_t objectsPerFrame,
    const uint32_t objectsPerFrameInc) {

#if !defined(__ANDROID__) && !defined(SMALL3D_IOS)
    window = 0;
#endif

    this->objectsPerFrame = objectsPerFrame;
    this->objectsPerFrameInc = objectsPerFrameInc;
    this->zNear = zNear;
    this->zFar = zFar;
    this->fieldOfView = fieldOfView;
    this->windowTitle = windowTitle;

    init(width, height, shadersPath);

    FT_Error ftError = FT_Init_FreeType(&library);

    if (ftError != 0) {
      throw std::runtime_error("Unable to initialise font system");
    }
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

  void Renderer::setCameraTransformation(const glm::mat4x4& transformation) {
    this->cameraTransformation = glm::inverse(transformation);
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
      throw std::runtime_error("Attempted x, y, z representation camera rotation retrieval, while having set the initial rotation by matrix.");
    }
    return this->cameraRotationXYZ;
  }

  int Renderer::getScreenWidth() {
    return realScreenWidth;
  }

  int Renderer::getScreenHeight() {
    return realScreenHeight;
  }

  Renderer& Renderer::getInstance(const std::string& windowTitle,
    const int width, const int height,
    const float fieldOfView,
    const float zNear, const float zFar,
    const std::string& shadersPath,
    const uint32_t objectsPerFrame,
    const uint32_t objectsPerFrameInc) {

    static Renderer instance(windowTitle, width, height, fieldOfView, zNear,
      zFar, shadersPath, objectsPerFrame, objectsPerFrameInc);

    return instance;
  }

  Renderer::~Renderer() {

    LOGDEBUG("Renderer destructor running");

    //With the following there is a crash sometimes when exiting the application.
    //destroyVulkan();

    for (auto& idFacePair : fontFaces) {
      FT_Done_Face(idFacePair.second);
    }

#ifdef __ANDROID__
    for (auto& asset : fontAssets) {
      AAsset_close(asset);
  }
#endif

    FT_Done_FreeType(library);

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

  void Renderer::generateTexture(const std::string& name, const Image& image) {
    this->generateTexture(name, image.getData(), image.getWidth(),
      image.getHeight(), false);
  }

  void Renderer::generateTexture(const std::string& name, const std::string& text,
    const glm::vec3& colour, const int fontSize,
    const std::string& fontPath, const bool replace) {

    std::string faceId = std::to_string(fontSize) + fontPath;

    auto idFacePair = fontFaces.find(faceId);
    FT_Face face;
    FT_Error error;

    if (idFacePair == fontFaces.end()) {
      std::string faceFullPath;

      faceFullPath = getBasePath() + fontPath;

      LOGDEBUG("Loading font from " + faceFullPath);
#ifdef __ANDROID__
      AAsset* asset = AAssetManager_open(small3d_android_app->activity->assetManager,
                                         faceFullPath.c_str(),
                                         AASSET_MODE_STREAMING);
      if (!asset) throw std::runtime_error("Opening asset " + faceFullPath +
        " has failed!");
      off_t length;
      length = AAsset_getLength(asset);
      const void* buffer = AAsset_getBuffer(asset);
      error = FT_New_Memory_Face(library, (const FT_Byte*)buffer,
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

  void Renderer::deleteTexture(const std::string& name) {
    auto nameTexturePair = textures.find(name);

    if (nameTexturePair != textures.end()) {

      deleteImageFromGPU(nameTexturePair->second);

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

  void Renderer::render(Model& model, const glm::vec3& offset,
    const glm::vec3& rotation,
    const glm::vec4& colour,
    const std::string& textureName,
    const bool perspective) {

    this->render(model, offset,
      glm::rotate(glm::mat4x4(1.0f), rotation.y, glm::vec3(0.0f, 1.0f, 0.0f)) *
      glm::rotate(glm::mat4x4(1.0f), rotation.x, glm::vec3(1.0f, 0.0f, 0.0f)) *
      glm::rotate(glm::mat4x4(1.0f), rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)),
      colour, textureName,
      perspective);

  }

  void Renderer::render(Model& model, const glm::vec3& offset,
    const glm::vec3& rotation,
    const std::string& textureName) {

    this->render(model, offset,
      glm::rotate(glm::mat4x4(1.0f), rotation.y, glm::vec3(0.0f, 1.0f, 0.0f)) *
      glm::rotate(glm::mat4x4(1.0f), rotation.x, glm::vec3(1.0f, 0.0f, 0.0f)) *
      glm::rotate(glm::mat4x4(1.0f), rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)),
      glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
      textureName);

  }

  void Renderer::render(Model& model, const glm::vec3& offset,
    const glm::mat4x4& rotation,
    const glm::vec4& colour,
    const std::string& textureName,
    const bool perspective) {

    

#if defined(DEBUG) || defined(_DEBUG) || !defined (NDEBUG)
    if (model.indexData.size() == 0 ||
      model.indexDataByteSize == 0 ||
      model.vertexData.size() == 0 ||
      model.vertexDataByteSize == 0) {
      throw std::runtime_error("Model to be rendered has some empty values!");
    }
#endif

    if (!(colourMemIndex < objectsPerFrame && modelPlacementMemIndex < objectsPerFrame)) {

      LOGDEBUG("Max objects per pass number(" + std::to_string(objectsPerFrame) +
        ") exceeded. Increasing number...");

      increaseObjectsPerFrame(objectsPerFrameInc);
      return;

    }

    if (model.renderIndex < memoryResetModelRenderIndex) {
      model.alreadyInGPU = false;
    }

    if (!model.alreadyInGPU) {
      model.renderIndex = nextModelRenderIndex;
      ++nextModelRenderIndex;

      // Send vertex data to GPU

      if (!vh_create_buffer(&model.positionBuffer,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        model.vertexDataByteSize,
        &model.positionBufferMemory,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
        throw std::runtime_error("Failed to create vertex buffer.");
      }

      allocatedBufferMemory.insert(std::make_pair(model.positionBuffer, model.positionBufferMemory));

      VkBuffer stagingBuffer;
      VkDeviceMemory stagingBufferMemory;

      if (vh_create_buffer(&stagingBuffer,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        model.vertexDataByteSize,
        &stagingBufferMemory,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
        void* stagingData;
        vkMapMemory(vh_logical_device, stagingBufferMemory, 0,
          VK_WHOLE_SIZE,
          0, &stagingData);
        memcpy(stagingData, &model.vertexData[0], model.vertexDataByteSize);
        vkUnmapMemory(vh_logical_device, stagingBufferMemory);

        vh_copy_buffer(stagingBuffer, model.positionBuffer,
          model.vertexDataByteSize);

        vh_destroy_buffer(stagingBuffer, stagingBufferMemory);
      }
      else {
        throw std::runtime_error("Failed to create the staging buffer"
          " for vertices.");
      }

      // Send index data

      if (!vh_create_buffer(&model.indexBuffer,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        model.indexDataByteSize,
        &model.indexBufferMemory,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
        throw std::runtime_error("Failed to create index buffer.");
      }

      allocatedBufferMemory.insert(std::make_pair(model.indexBuffer, model.indexBufferMemory));

      if (vh_create_buffer(&stagingBuffer,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        model.indexDataByteSize,
        &stagingBufferMemory,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
        void* stagingData;
        vkMapMemory(vh_logical_device, stagingBufferMemory, 0,
          VK_WHOLE_SIZE,
          0, &stagingData);
        memcpy(stagingData, &model.indexData[0], model.indexDataByteSize);
        vkUnmapMemory(vh_logical_device, stagingBufferMemory);

        vh_copy_buffer(stagingBuffer, model.indexBuffer,
          model.indexDataByteSize);

        vh_destroy_buffer(stagingBuffer, stagingBufferMemory);
      }
      else {
        throw std::runtime_error("Failed to create the staging"
          " buffer for indices.");
      }

      // The following buffers are created with 0 values if the corresponding
      // data does not exist, otherwise there can be issues, especially
      // with MoltenVK

      // Send normals data

      auto nrByteSize = model.normalsDataByteSize == 0 ? (model.vertexDataByteSize / 4) * 3 : model.normalsDataByteSize;

      if (!vh_create_buffer(&model.normalsBuffer,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        nrByteSize,
        &model.normalsBufferMemory,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
        throw std::runtime_error("Failed to create normals buffer.");
      }

      allocatedBufferMemory.insert(std::make_pair(model.normalsBuffer, model.normalsBufferMemory));

      if (vh_create_buffer(&stagingBuffer,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        nrByteSize,
        &stagingBufferMemory,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
        void* normalsData;
        vkMapMemory(vh_logical_device, stagingBufferMemory, 0,
          VK_WHOLE_SIZE,
          0, &normalsData);


        if (model.normalsDataByteSize != 0) {
          memcpy(normalsData, &model.normalsData[0], model.normalsDataByteSize);
        }
        else {
          memset(normalsData, 0, nrByteSize);
        }

        vkUnmapMemory(vh_logical_device, stagingBufferMemory);

        vh_copy_buffer(stagingBuffer, model.normalsBuffer,
          nrByteSize);

        vh_destroy_buffer(stagingBuffer, stagingBufferMemory);
      }
      else {
        throw std::runtime_error("Failed to create the staging"
          " buffer for indices.");
      }

      auto uvByteSize = model.textureCoordsDataByteSize == 0 ? model.vertexDataByteSize / 2 : model.textureCoordsDataByteSize;

      if (!vh_create_buffer(&model.uvBuffer,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        uvByteSize,
        &model.uvBufferMemory,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
        throw std::runtime_error("Failed to create uv buffer.");
      }

      allocatedBufferMemory.insert(std::make_pair(model.uvBuffer, model.uvBufferMemory));

      if (vh_create_buffer(&stagingBuffer,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        uvByteSize,
        &stagingBufferMemory,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
        void* uvData;
        vkMapMemory(vh_logical_device, stagingBufferMemory, 0,
          VK_WHOLE_SIZE,
          0, &uvData);

        if (model.textureCoordsDataByteSize != 0) {
          memcpy(uvData, &model.textureCoordsData[0],
            uvByteSize);
        }
        else {
          memset(uvData, 0, uvByteSize);
        }

        vkUnmapMemory(vh_logical_device, stagingBufferMemory);

        vh_copy_buffer(stagingBuffer, model.uvBuffer,
          uvByteSize);

        vh_destroy_buffer(stagingBuffer, stagingBufferMemory);
      }
      else {
        throw std::runtime_error("Failed to create the staging"
          " buffer for texture coordinates.");
      }

      auto jointByteSize = model.jointDataByteSize == 0 ? model.vertexDataByteSize / 4 :
        model.jointDataByteSize;

      if (!vh_create_buffer(&model.jointBuffer,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        jointByteSize,
        &model.jointBufferMemory,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
        throw std::runtime_error("Failed to create joint buffer.");
      }

      allocatedBufferMemory.insert(std::make_pair(model.jointBuffer, model.jointBufferMemory));

      if (vh_create_buffer(&stagingBuffer,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        jointByteSize,
        &stagingBufferMemory,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
        void* jointData;
        vkMapMemory(vh_logical_device, stagingBufferMemory, 0,
          VK_WHOLE_SIZE,
          0, &jointData);

        if (model.jointDataByteSize > 0) {
          memcpy(jointData, &model.jointData[0],
            model.jointDataByteSize);
        }
        else {
          memset(jointData, 0, jointByteSize);
        }
        vkUnmapMemory(vh_logical_device, stagingBufferMemory);

        vh_copy_buffer(stagingBuffer, model.jointBuffer,
          jointByteSize);

        vh_destroy_buffer(stagingBuffer, stagingBufferMemory);
      }
      else {
        throw std::runtime_error("Failed to create the staging"
          " buffer for joints.");
      }

      auto weightByteSize = model.weightDataByteSize == 0 ? model.vertexDataByteSize :
        model.weightDataByteSize;
      if (!vh_create_buffer(&model.weightBuffer,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        weightByteSize,
        &model.weightBufferMemory,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
        throw std::runtime_error("Failed to create weight buffer.");
      }

      allocatedBufferMemory.insert(std::make_pair(model.weightBuffer, model.weightBufferMemory));

      if (vh_create_buffer(&stagingBuffer,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        weightByteSize,
        &stagingBufferMemory,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
        void* weightData;
        vkMapMemory(vh_logical_device, stagingBufferMemory, 0,
          VK_WHOLE_SIZE,
          0, &weightData);

        if (model.weightDataByteSize > 0) {
          memcpy(weightData, &model.weightData[0],
            model.weightDataByteSize);
        }
        else {
          memset(weightData, 0, weightByteSize);
        }

        vkUnmapMemory(vh_logical_device, stagingBufferMemory);

        vh_copy_buffer(stagingBuffer, model.weightBuffer,
          weightByteSize);

        vh_destroy_buffer(stagingBuffer, stagingBufferMemory);
      }
      else {
        throw std::runtime_error("Failed to create the staging"
          " buffer for weights.");
      }

      model.alreadyInGPU = true;
    }

    if (textureName != "") {
      // "Disable" colour since there is a texture
      setColourBuffer(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), colourMemIndex);
      model.textureName = textureName;
    }
    else {
      // If there is no texture, use the given colour
      setColourBuffer(colour, colourMemIndex);
      model.textureName = "blank";
    }

    model.colourMemIndex = colourMemIndex;
    ++colourMemIndex;

    model.perspective = perspective;

    transform(model, offset, rotation, modelPlacementMemIndex);
    model.placementMemIndex = modelPlacementMemIndex;
    ++modelPlacementMemIndex;

    nextModelsToDraw.push_back(model);

  }

  void Renderer::render(Model& model, const glm::vec3& position,
    const glm::mat4x4& rotation,
    const std::string& textureName) {
    this->render(model, position, rotation, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
      textureName);
  }

  void Renderer::render(Model& model, const std::string& textureName,
    const bool perspective) {
    this->render(model, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
      glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
      textureName, perspective);
  }

  void Renderer::render(Model& model, const glm::vec4& colour,
    const bool perspective) {
    this->render(model, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
      colour, "", perspective);
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

  void Renderer::clearBuffers(Model& model) {
    vkDeviceWaitIdle(vh_logical_device);
    if (model.alreadyInGPU) {
      vh_destroy_buffer(model.positionBuffer, model.positionBufferMemory);
      allocatedBufferMemory.erase(model.positionBuffer);
      vh_destroy_buffer(model.indexBuffer, model.indexBufferMemory);
      allocatedBufferMemory.erase(model.indexBuffer);
      vh_destroy_buffer(model.normalsBuffer, model.normalsBufferMemory);
      allocatedBufferMemory.erase(model.normalsBuffer);
      vh_destroy_buffer(model.uvBuffer, model.uvBufferMemory);
      allocatedBufferMemory.erase(model.uvBuffer);
      vh_destroy_buffer(model.jointBuffer, model.jointBufferMemory);
      allocatedBufferMemory.erase(model.jointBuffer);
      vh_destroy_buffer(model.weightBuffer, model.weightBufferMemory);
      allocatedBufferMemory.erase(model.weightBuffer);
      model.alreadyInGPU = false;
    }
  }

  void Renderer::clearBuffers(SceneObject& sceneObject) {
    for (auto& model : *sceneObject.models) {
      clearBuffers(model);
    }
  }

  void Renderer::setBackgroundColour(const glm::vec4& colour) {
    backgroundColour = colour;
    vh_clear_colour.float32[0] = colour.r;
    vh_clear_colour.float32[1] = colour.g;
    vh_clear_colour.float32[2] = colour.b;
    vh_clear_colour.float32[3] = colour.a;
  }

  void Renderer::swapBuffers() {

    uint32_t imageIndexNotNeeded = 0;

    vh_acquire_next_image(pipelineIndex,
      &imageIndexNotNeeded, &currentFrameIndex);
    
    vh_wait_gpu_cpu_fence(currentFrameIndex);
    vh_destroy_draw_command_buffer(&commandBuffer[currentFrameIndex]);

    modelPlacementMemIndex = 0;
    colourMemIndex = 0;

    setWorldDetails(true);
    setWorldDetails(false);
    setLightIntensity();

    // Updating object positioning 
    void* modelPlacementData;
    vkMapMemory(vh_logical_device,
      renderModelPlacementBuffersDynamicMemory[currentFrameIndex],
      0, uboModelPlacementDynamicSize, 0, &modelPlacementData);
    memcpy(modelPlacementData, uboModelPlacementDynamic, uboModelPlacementDynamicSize);
    vkUnmapMemory(vh_logical_device,
      renderModelPlacementBuffersDynamicMemory[currentFrameIndex]);

    // Updating world details
    void* worldDetailsData;
    vkMapMemory(vh_logical_device,
      worldDetailsBuffersDynamicMemory[currentFrameIndex],
      0, uboWorldDetailsDynamicSize, 0, &worldDetailsData);
    memcpy(worldDetailsData, uboWorldDetailsDynamic, uboWorldDetailsDynamicSize);
    vkUnmapMemory(vh_logical_device,
      worldDetailsBuffersDynamicMemory[currentFrameIndex]);

    // Updating colour
    void* colourData;
    vkMapMemory(vh_logical_device,
      colourBuffersDynamicMemory[currentFrameIndex],
      0, uboColourDynamicSize, 0, &colourData);
    memcpy(colourData, uboColourDynamic, uboColourDynamicSize);
    vkUnmapMemory(vh_logical_device,
      colourBuffersDynamicMemory[currentFrameIndex]);

    // All of the above is bound here.
    updateDescriptorSets();

    vh_begin_draw_command_buffer(&commandBuffer[currentFrameIndex]);

    for (auto& model : nextModelsToDraw) {
      vh_bind_pipeline_to_command_buffer(pipelineIndex,
        &commandBuffer[currentFrameIndex]);
      bindBuffers(commandBuffer[currentFrameIndex], model);

      if (!model.perspective) {
        vh_clear_depth_image(&commandBuffer[currentFrameIndex]);
      }

      recordDrawCommand(commandBuffer[currentFrameIndex],
        vh_pipeline_layout[pipelineIndex],
        model, currentFrameIndex, model.perspective);
    }

    vh_end_draw_command_buffer(&commandBuffer[currentFrameIndex]);

    vh_draw(&commandBuffer[currentFrameIndex]);

    vh_present_next_image();

    nextModelsToDraw.clear();

    for (auto& model : garbageModels) {
      clearBuffers(model);
    }
    garbageModels.clear();

  }

  void Renderer::setupVulkan() {


#if defined(__ANDROID__)
    const char* exts[2];

    exts[0] = VK_KHR_SURFACE_EXTENSION_NAME;
    exts[1] = VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;

    uint32_t num = 2;

    LOGDEBUG("Creating Vulkan instance...");

    if (!vh_create_instance(windowTitle.c_str(), exts, num)) {
      throw std::runtime_error("Failed to create Vulkan instance.");
    }

    VkAndroidSurfaceCreateInfoKHR sci;
    sci.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    sci.pNext = nullptr;
    sci.flags = 0;
    sci.window = small3d_android_app->window;
    LOGDEBUG("Creating surface...");
    if (vkCreateAndroidSurfaceKHR(vh_instance, &sci, nullptr, &vh_surface) !=
      VK_SUCCESS) {
      throw std::runtime_error("Could not create surface.");
    }
#elif defined(SMALL3D_IOS)

    const char* exts[2];

    exts[0] = VK_KHR_SURFACE_EXTENSION_NAME;
    exts[1] = "VK_MVK_ios_surface"; //VK_MVK_IOS_SURFACE_EXTENSION_NAME;
    //This is not found on iOS and returns an error.
    //The VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR is used
    //for the instance creation without any issue though..
    //exts[2] = "VK_KHR_portability_enumeration";

    uint32_t num = 2;

    LOGDEBUG("Creating Vulkan instance...");

    if (!vh_create_instance(windowTitle.c_str(), exts, num)) {
      throw std::runtime_error("Failed to create Vulkan instance.");
    }

    if (!create_ios_surface(vh_instance, &vh_surface)) {
      throw std::runtime_error("Could not create surface.");
    }

#else

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
#if defined(__APPLE__)
    glfwExtensions[glfwExtensionCount] =  "VK_KHR_portability_enumeration";
    ++glfwExtensionCount;
#endif
    std::string requiredExtensions = "GLFW required extensions (";
    requiredExtensions += std::to_string(glfwExtensionCount) + ")";
    LOGDEBUG(requiredExtensions);

    for (uint32_t n = 0; n < glfwExtensionCount; n++) {
      LOGDEBUG(glfwExtensions[n]);
    }
    printf("\n\r");

    if (!vh_create_instance(windowTitle.c_str(), glfwExtensions,
      glfwExtensionCount)) {
      throw std::runtime_error("Failed to create Vulkan instance.");
    }

    if (glfwCreateWindowSurface(vh_instance, window, NULL, &vh_surface) !=
      VK_SUCCESS) {
      throw std::runtime_error("Could not create surface.");
    }
#endif

    if (!vh_init(MAX_FRAMES_PREPARED)) {
      throw std::runtime_error("Could not initialise Vulkan.");
    }

    if (realScreenWidth == 0) realScreenWidth = 1;
    if (realScreenHeight == 0) realScreenHeight = 1;
    vh_set_width_height(realScreenWidth, realScreenHeight);

    if (!vh_create_sync_objects()) {
      throw std::runtime_error("Failed to create sync objects.");
    }

    LOGDEBUG("Creating swapchain...");

    if (!vh_create_swapchain()) {
      throw std::runtime_error("Failed to create swapchain.");
    }

    LOGDEBUG("Creating descriptor pool...");

    createDescriptorPool();
    allocateDescriptorSets();
    // Shader with joints does not compile on some Android GPUs
    // like Adreno (Qualcomm)
#if !defined(__ANDROID__)
    std::string vertexShaderPath = this->shadersPath +
      "perspectiveMatrixLightedShader.spv";
#else
    std::string vertexShaderPath = this->shadersPath +
      "perspectiveMatrixLightedShaderNoJoints.spv";
#endif
    std::string fragmentShaderPath = this->shadersPath +
      "textureShader.spv";

    if (!vh_create_sampler(&textureSampler)) {
      throw std::runtime_error("Failed to create the sampler!");
    }

    if (!vh_create_pipeline(vertexShaderPath.c_str(), fragmentShaderPath.c_str(),
      setInputStateCallback, setPipelineLayoutCallback,
      &pipelineIndex)) {
      throw std::runtime_error("Could not create the Vulkan pipeline!");
    }

    allocateDynamicBuffers();

    // Re-generate textures if this is a re-creation of the pipeline
    for (auto& t : textures) {
      generateTexture(t.first, nullptr, 0, 0, false);
    }

    // If this is a re-creation of the pipeline, models have to be put
    // back into memory.
    memoryResetModelRenderIndex = nextModelRenderIndex;
    currentFrameIndex = 0;

    vh_clear_colour.float32[0] = backgroundColour.r;
    vh_clear_colour.float32[1] = backgroundColour.g;
    vh_clear_colour.float32[2] = backgroundColour.b;
    vh_clear_colour.float32[3] = backgroundColour.a;
  }

  void Renderer::destroyVulkan() {

    vkDeviceWaitIdle(vh_logical_device);
    
    for (uint32_t idx = 0; idx < MAX_FRAMES_PREPARED; ++idx) {
      vh_destroy_draw_command_buffer(&commandBuffer[idx]);
      commandBuffer[idx] = VK_NULL_HANDLE;
    }

    LOGDEBUG("Destroying buffers...");

    for (auto& b : allocatedBufferMemory) {
      vh_destroy_buffer(b.first, b.second);
    }

    allocatedBufferMemory.clear();

    nextModelsToDraw.clear();

    garbageModels.clear();

    for (auto& t : textures) {
      LOGDEBUG("Deleting texture " + t.first);
      deleteImageFromGPU(t.second);
    }

    vh_destroy_pipeline(pipelineIndex);

    vkDestroySampler(vh_logical_device, textureSampler, NULL);

    destroyDescriptorSets();

    destroyDescriptorPool();

    destroyDynamicBuffers();

    vh_destroy_swapchain();

    vh_destroy_sync_objects();

    LOGDEBUG("Destroying surface.\n\r");

    vkDestroySurfaceKHR(vh_instance, vh_surface, NULL);

    vh_shutdown();

    pipelineIndex = 100;
  }

}
