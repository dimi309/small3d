/*
 *  vkzos.c
 *
 *  Created on: 2018/05/01
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */
#include "vkzos.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef int BOOL;
#define TRUE 1
#define FALSE 0

#ifdef __ANDROID__
#include <android/log.h>
struct android_app* vkz_android_app;
#endif

#ifndef NDEBUG
#ifdef __ANDROID__
#define LOGDEBUG0(x) __android_log_write(ANDROID_LOG_DEBUG, "vkzos", x)
#define LOGDEBUG1(x, y) __android_log_print(ANDROID_LOG_DEBUG, "vkzos", x, y)
#define LOGDEBUG2(x, y, z) __android_log_print(ANDROID_LOG_DEBUG, "vkzos", x, y, z)

#else

#define LOGDEBUG0(x) printf(x); printf("\n\r")
#define LOGDEBUG1(x, y) printf(x, y); printf("\n\r")
#define LOGDEBUG2(x, y, z) printf(x, y, z); printf("\n\r")

#endif

uint32_t numValidationLayers = 3; // Set to 4 to enable VK_LAYER_MESA_overlay
const char* vl[4] = {
  "VK_LAYER_LUNARG_standard_validation",
  "VK_LAYER_MESA_device_select",
  "VK_LAYER_KHRONOS_validation",
  "VK_LAYER_MESA_overlay"
};

#else
#define LOGDEBUG0(x)
#define LOGDEBUG1(x, y) 
#define LOGDEBUG2(x, y, z)

uint32_t numValidationLayers = 0;

#endif

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugReportFlagsEXT flags,
  VkDebugReportObjectTypeEXT objType,
  uint64_t obj,
  size_t location,
  int32_t code,
  const char* layerPrefix,
  const char* msg,
  void* userData) {

  LOGDEBUG1("validation layer: %s", msg);

  return VK_FALSE;
}

static uint32_t max_frames_prepared;

static VkDebugReportCallbackEXT callback;

static BOOL VK_KHR_get_physical_device_properties2_supported = FALSE;

static BOOL VK_KHR_portability_subset_supported = FALSE;

static BOOL debug_callback_created = FALSE;
static BOOL instance_created = FALSE;
static BOOL logical_device_created = FALSE;

static BOOL swapchain_created = FALSE;
static BOOL swapchain_image_views_created = FALSE;

static uint32_t vkz_width;
static uint32_t vkz_height;

static const char* validation_layers[5];
static uint32_t validation_layer_count = 0;

VkInstance vkz_instance;
VkSurfaceKHR vkz_surface;
VkPhysicalDevice vkz_physical_device;
VkDevice vkz_logical_device;
VkClearColorValue vkz_clear_colour;

static int vkz_graphics_family_index = -1;
static int vkz_present_family_index = -1;
static VkQueue vkz_graphics_queue;
static VkQueue vkz_present_queue;

typedef struct {
  VkSurfaceCapabilitiesKHR capabilities;
  uint32_t formatCount;
  VkSurfaceFormatKHR* formats;
  uint32_t presentModeCount;
  VkPresentModeKHR* presentModes;

} swapchain_support_struct;

uint32_t vkz_swapchain_image_count;

static swapchain_support_struct vkz_swapchain_support_details;

static VkSurfaceFormatKHR vkz_surface_format;
static VkPresentModeKHR vkz_present_mode;
static VkExtent2D vkz_swap_extent;
static VkSwapchainKHR vkz_swapchain;
static VkImage* vkz_swapchain_images = NULL;
static VkImageView* vkz_swapchain_image_views = NULL;

static VkFramebuffer* framebuffers;
static VkRenderPass render_pass;

static VkCommandPool command_pool;

static VkImage depth_image;
static VkFormat depth_image_format;
static VkDeviceMemory depth_image_memory;
static VkImageView depth_image_view;

static uint32_t next_image_index;

VkPipelineLayout* vkz_pipeline_layout;

typedef struct {
  BOOL deleted;
  char* vertex_shader_path;
  char* fragment_shader_path;
  VkShaderModule vertex_shader_module;
  VkShaderModule fragment_shader_module;
  VkPipeline pipeline;
  int (*set_input_state_function)(VkPipelineVertexInputStateCreateInfo*);
  int (*set_pipeline_layout_function)(VkPipelineLayoutCreateInfo*);
} pipeline_system_struct;

uint32_t pipeline_system_count = 0;
pipeline_system_struct* pipeline_systems = NULL;

static VkFence* gpu_cpu_fence;
static VkSemaphore *draw_semaphore, *acquire_semaphore;
static uint32_t frame_index = 0;

void vkz_wait_gpu_cpu_fence(uint32_t idx) {
  if (gpu_cpu_fence[idx] == VK_NULL_HANDLE) {
    LOGDEBUG0("Waiting on gpu_cpu_fence that is null!");
  }
  VkResult rwait;
  do {
    rwait = vkWaitForFences(vkz_logical_device, 1,
      &gpu_cpu_fence[idx],
      VK_TRUE, UINT64_MAX);
  } while (rwait == VK_TIMEOUT);
}

int vkz_create_instance(const char* application_name,
  const char** enabled_extension_names,
  size_t enabled_extension_count) {

  LOGDEBUG0("vkz_create_instance requested extensions:");
  for (int i = 0; i < enabled_extension_count; ++i) {
    LOGDEBUG1("%s", enabled_extension_names[i]);
  }

  uint32_t property_count = 0;

  vkEnumerateInstanceExtensionProperties(NULL, &property_count,
    NULL);
  if (property_count > 0) {
    VkExtensionProperties* extension_properties = malloc(sizeof(VkExtensionProperties) * property_count);

    vkEnumerateInstanceExtensionProperties(NULL, &property_count,
      extension_properties);
    LOGDEBUG0("Checking for requested extension support...");
    for (uint32_t i1 = 0; i1 < enabled_extension_count; ++i1) {
      BOOL ext_found = FALSE;
      for (uint32_t i = 0; i < property_count; ++i) {
        if (strcmp(enabled_extension_names[i1], extension_properties[i].extensionName) == 0) {
          ext_found = TRUE;
        }
      }
      if (!ext_found) {
        LOGDEBUG1("%s not supported! Cannot create Vulkan instance.", enabled_extension_names[i1]);
        return 0;
      }
    }

    VK_KHR_get_physical_device_properties2_supported = FALSE;
    LOGDEBUG0("Searching through instance supported extensions...");
    for (uint32_t i = 0; i < property_count; ++i) {
      if (strcmp("VK_KHR_get_physical_device_properties2", extension_properties[i].extensionName) == 0) {
        VK_KHR_get_physical_device_properties2_supported = TRUE;
      }
    }
    free(extension_properties);
  }


  VkApplicationInfo ai;
  memset(&ai, 0, sizeof(VkApplicationInfo));
  ai.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  ai.pApplicationName = application_name;
  ai.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  ai.pEngineName = "vkzos";
  ai.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  ai.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo ci;
  memset(&ci, 0, sizeof(VkInstanceCreateInfo));
  ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  ci.pApplicationInfo = &ai;

#ifndef NDEBUG
  uint32_t lc = 0;
  VkLayerProperties* lp = NULL;

  vkEnumerateInstanceLayerProperties(&lc, NULL);
  lp = malloc(sizeof(VkLayerProperties) * lc);
  validation_layer_count = 0;
  if (lp) {
    memset(lp, 0, sizeof(VkLayerProperties) * lc);
    vkEnumerateInstanceLayerProperties(&lc, lp);
    for (uint32_t i = 0; i < numValidationLayers; i++) {
      LOGDEBUG1("Looking for %s", vl[i]);
      for (uint32_t n = 0; n < lc; ++n) {      
        // Disable C6385 warning in Visual Studio as it probably gives a false positive here.
        // see https://stackoverflow.com/questions/59649678/warning-c6385-in-visual-studio
#pragma warning(push)
#pragma warning(disable:6385)
        if (strcmp(lp[n].layerName, vl[i]) == 0) {
          LOGDEBUG1("Layer %s exists! Will enable...\n", vl[i]);
          validation_layers[validation_layer_count] = vl[i];
          ++validation_layer_count;
        }
#pragma warning(pop)
      }
    }
  }
  if (validation_layer_count == 0) {
    LOGDEBUG0("No validation layers found.");
  }

  ci.enabledExtensionCount = (uint32_t)enabled_extension_count;
  ci.ppEnabledExtensionNames = enabled_extension_names;

  // This would normally be enabled_extension_count + 1 but Visual
  // Studio doesn't like such qualifiers
  const char* allExtensionNames[10];
  uint32_t allExtensionCount = (uint32_t)enabled_extension_count;

  for (uint32_t n = 0; n < enabled_extension_count; n++) {
    allExtensionNames[n] = enabled_extension_names[n];
  }

  if (VK_KHR_get_physical_device_properties2_supported) {
    allExtensionNames[allExtensionCount] = "VK_KHR_get_physical_device_properties2";
    ++allExtensionCount;
  }

  if (validation_layer_count > 0) {

    ci.enabledLayerCount = validation_layer_count;
    ci.ppEnabledLayerNames = validation_layers;

    allExtensionNames[allExtensionCount] =
      VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
    ++allExtensionCount;

  }

  ci.enabledExtensionCount = allExtensionCount;
  ci.ppEnabledExtensionNames = allExtensionNames;

#else
  ci.enabledExtensionCount = enabled_extension_count;
  ci.ppEnabledExtensionNames = enabled_extension_names;
#endif

  int success = 1;
  VkResult result = vkCreateInstance(&ci, NULL, &vkz_instance);
  if (result != VK_SUCCESS) {
    success = 0;
    LOGDEBUG1("Failed to create Vulkan instance. Error: %d", result);
  }
  else {
    LOGDEBUG0("Created Vulkan instance.");
    instance_created = TRUE;
#if !defined(NDEBUG)
    if (validation_layer_count > 0) {
      VkDebugReportCallbackCreateInfoEXT dcci;
      dcci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
      dcci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
        VK_DEBUG_REPORT_WARNING_BIT_EXT;
      dcci.pfnCallback = debugCallback;
      dcci.pNext = NULL;

      PFN_vkCreateDebugReportCallbackEXT dcCreate =
        (PFN_vkCreateDebugReportCallbackEXT)
        vkGetInstanceProcAddr(vkz_instance, "vkCreateDebugReportCallbackEXT");

      if (dcCreate) {
        if (dcCreate(vkz_instance, &dcci, NULL, &callback) != VK_SUCCESS) {
          LOGDEBUG0("Failed to create debug report callback!");
          success = 0;
        }
        else {
          LOGDEBUG0("Created debug report callback.");
          debug_callback_created = TRUE;
        }
      }
      else LOGDEBUG0("Could not get debug report creation function address!");
    }
#endif
  }

#ifndef NDEBUG

  if (lp) {
    free(lp);
    lp = NULL;
  }
#endif

  return success;
}

int retrieve_swapchain_support_details(VkPhysicalDevice device) {
  memset(&vkz_swapchain_support_details.capabilities, 0,
    sizeof(VkSurfaceCapabilitiesKHR));
  vkz_swapchain_support_details.formatCount = 0;
  vkz_swapchain_support_details.formats = NULL;
  vkz_swapchain_support_details.presentModeCount = 0;
  vkz_swapchain_support_details.presentModes = NULL;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, vkz_surface,
    &vkz_swapchain_support_details.capabilities);
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, vkz_surface,
    &vkz_swapchain_support_details.formatCount,
    NULL);
  if (vkz_swapchain_support_details.formatCount > 0) {
    vkz_swapchain_support_details.formats =
      malloc(vkz_swapchain_support_details.formatCount *
        sizeof(VkSurfaceFormatKHR));
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, vkz_surface,
      &vkz_swapchain_support_details.formatCount,
      vkz_swapchain_support_details.formats);

  }
  else {
    LOGDEBUG0("No surface formats found for physical device!");
    return 0;
  }

  vkGetPhysicalDeviceSurfacePresentModesKHR(device, vkz_surface,
    &vkz_swapchain_support_details.presentModeCount,
    NULL);
  if (vkz_swapchain_support_details.presentModeCount > 0) {
    vkz_swapchain_support_details.presentModes =
      malloc(vkz_swapchain_support_details.presentModeCount *
        sizeof(VkPresentModeKHR));
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, vkz_surface,
      &vkz_swapchain_support_details.presentModeCount,
      vkz_swapchain_support_details.presentModes);
  }
  else {

    LOGDEBUG0("No present modes found for physical device!");
    return 0;
  }

  return 1;
}

int select_surface_format() {
  LOGDEBUG1("Selecting surface format. formatCount: %d",
    vkz_swapchain_support_details.formatCount);
  if (!(vkz_swapchain_support_details.formatCount == 1 &&
    vkz_swapchain_support_details.formats[0].format ==
    VK_FORMAT_UNDEFINED)) {
    BOOL found = FALSE;
    for (uint32_t n = 0; n < vkz_swapchain_support_details.formatCount; n++) {
      LOGDEBUG2("Found format %d with colorSpace %d",
        vkz_swapchain_support_details.formats[n].format,
        vkz_swapchain_support_details.formats[n].colorSpace);
      if (vkz_swapchain_support_details.formats[n].format ==
        VK_FORMAT_B8G8R8A8_UNORM &&
        vkz_swapchain_support_details.formats[n].colorSpace ==
        VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
        found = TRUE;
        break;
      }
    }
    if (!found) {
      LOGDEBUG2("Preferred surface format not supported. Using an existing"
        " one. (Format %d colorSpace %d)",
        vkz_swapchain_support_details.formats[0].format,
        vkz_swapchain_support_details.formats[0].colorSpace);
      vkz_surface_format.format =
        vkz_swapchain_support_details.formats[0].format;
      vkz_surface_format.colorSpace =
        vkz_swapchain_support_details.formats[0].colorSpace;
      return 1;
    }
  }

  LOGDEBUG0("Preferred surface format supported.");
  vkz_surface_format.format = VK_FORMAT_B8G8R8A8_UNORM;
  vkz_surface_format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

  return 1;
}

int select_present_mode() {

  vkz_present_mode = VK_PRESENT_MODE_FIFO_KHR;

  BOOL found = FALSE;
  for (uint32_t n = 0; n < vkz_swapchain_support_details.presentModeCount; n++) {
    if (vkz_swapchain_support_details.presentModes[n] ==
      VK_PRESENT_MODE_MAILBOX_KHR) {
      vkz_present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
      LOGDEBUG0("Mailbox present mode supported.");
      found = TRUE;
      break;
    }
  }

  // IMPORTANT NOTE: In the absence of immediate khr, with an AMD Raderon R5
  // (possibly m330) which didn't support mailbox and thus made the code
  // default to fifo, a Windows 10 machine would completely freeze. Only a
  // reboot would fix the situation. On that particular machine, even the lunarg
  // examples freeze the system, whereas adding the option to use immediate khr
  // here below has allowed this code to work.
  if (!found) {
    for (uint32_t n = 0; n < vkz_swapchain_support_details.presentModeCount;
      n++) {
      if (vkz_swapchain_support_details.presentModes[n] ==
        VK_PRESENT_MODE_IMMEDIATE_KHR) {
        vkz_present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
        LOGDEBUG0("Immediate present mode supported.");
        found = TRUE;
        break;
      }
    }
  }

  if (!found) {
    LOGDEBUG0("Mailbox and immediate present modes not supported,"
      " so using FIFO.");
  }

  return 1;
}

int select_physical_device() {
  int success = FALSE;
  uint32_t numDevices = 0;
  VkPhysicalDevice* pds = 0;
  VkExtensionProperties* deviceExtensions = NULL;
  uint32_t deviceExtensionCount = 0;

  vkEnumeratePhysicalDevices(vkz_instance, &numDevices, NULL);

  if (numDevices > 0) {
    pds = malloc(sizeof(VkPhysicalDevice) * numDevices);
    if (!pds) return 0;
    vkEnumeratePhysicalDevices(vkz_instance, &numDevices, pds);

    LOGDEBUG1("Number of devices: %d - checking features...", numDevices);

    for (uint32_t n = 0; n < numDevices; n++) {

      VkPhysicalDeviceProperties dp;
      VkPhysicalDeviceFeatures df;

      vkGetPhysicalDeviceProperties(pds[n], &dp);
      vkGetPhysicalDeviceFeatures(pds[n], &df);

      LOGDEBUG1("Checking physical device %s...", dp.deviceName);

      if (df.geometryShader) {

        LOGDEBUG1("%s", "Geometry shader support present.");
      }
      else {
        LOGDEBUG0("Geometry shader support NOT present.");
      }

      vkEnumerateDeviceExtensionProperties(pds[n], NULL, &deviceExtensionCount,
        NULL);

      if (deviceExtensionCount) {
        if (deviceExtensions) {
          free(deviceExtensions);
        }
        deviceExtensions = malloc(sizeof(VkExtensionProperties) *
          deviceExtensionCount);
        vkEnumerateDeviceExtensionProperties(pds[n], NULL,
          &deviceExtensionCount,
          deviceExtensions);
      }

      BOOL swapchainSupported = FALSE;
      BOOL supportDetailsOk = FALSE;

      if (deviceExtensions) {
        for (uint32_t n1 = 0; n1 < deviceExtensionCount; n1++) {
          swapchainSupported = strcmp(VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            deviceExtensions[n1].extensionName) == 0;
          if (swapchainSupported && retrieve_swapchain_support_details(pds[n])) {
            LOGDEBUG0("The device supports the swapchain extension and"
              " support details are ok.");
            supportDetailsOk = TRUE;
            break;
          }
        }
      }

      if (!supportDetailsOk) {
        LOGDEBUG0(
          "Swapcain not supported or failed to retrieve support detais!");

      }
      else {

        vkz_physical_device = pds[n];

        LOGDEBUG1("Found good physical device: %s", dp.deviceName);

        success = select_surface_format() && select_present_mode();

        VK_KHR_portability_subset_supported = FALSE;
        LOGDEBUG0("Searching through extensions supported by physical device...");
        for (uint32_t n1 = 0; n1 < deviceExtensionCount; n1++) {
          if (strcmp("VK_KHR_portability_subset", deviceExtensions[n1].extensionName) == 0) {
            VK_KHR_portability_subset_supported = TRUE;
            LOGDEBUG0("The device supports the VK_KHR_portability_subset extension. It will be enabled.");
          }
        }
        break;
      }
    }
  }

  if (!success)
    LOGDEBUG0("Physical device selection failed!");

  if (pds) free(pds);
  if (deviceExtensions) free(deviceExtensions);

  return success;
}

int select_queue_families() {
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(vkz_physical_device,
    &queueFamilyCount,
    NULL);
  VkQueueFamilyProperties* queueFamilyProperties =
    malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);

  vkGetPhysicalDeviceQueueFamilyProperties(vkz_physical_device,
    &queueFamilyCount,
    queueFamilyProperties);

  BOOL found_graphics = FALSE;
  BOOL found_present = FALSE;

  if (queueFamilyProperties) {
    for (uint32_t n = 0; n < queueFamilyCount; n++) {
      // Disable C6385 warning in Visual Studio as it probably gives a false positive here.
      // see https://stackoverflow.com/questions/59649678/warning-c6385-in-visual-studio
#pragma warning(push)
#pragma warning(disable:6385)
      if (queueFamilyProperties[n].queueCount > 0 &&
        queueFamilyProperties[n].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        vkz_graphics_family_index = n;
        found_graphics = TRUE;
        LOGDEBUG1("Found graphics queue family index: %d",
          vkz_graphics_family_index);
        break;
      }
#pragma warning(pop)
    }

    if (!found_graphics) {
      LOGDEBUG0("Could not find graphics queue family!");
    }

    for (uint32_t n = 0; n < queueFamilyCount; n++) {
      if (queueFamilyProperties[n].queueCount > 0) {

        VkBool32 presentSupport = FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(vkz_physical_device, n, vkz_surface,
          &presentSupport);

        if (presentSupport) {
          vkz_present_family_index = n;
          found_present = TRUE;
          LOGDEBUG1("Found present queue family index: %d",
            vkz_present_family_index);
          break;
        }
      }
    }
    if (!found_present) {
      LOGDEBUG0("Could not find present queue family!");
    }
  }

  free(queueFamilyProperties);
  return (found_graphics && found_present);
}

int create_logical_device() {

  VkDeviceQueueCreateInfo* qci;

  if (vkz_graphics_family_index == vkz_present_family_index) {
    qci = malloc(sizeof(VkDeviceQueueCreateInfo));
  }
  else {
    qci = malloc(sizeof(VkDeviceQueueCreateInfo) * 2);
  }

  if (!qci) return 0;

  memset(qci, 0, sizeof(VkDeviceQueueCreateInfo));
  qci[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  qci[0].queueFamilyIndex = vkz_graphics_family_index;
  qci[0].queueCount = 1;
  float queuePriority = 1.0f;
  qci[0].pQueuePriorities = &queuePriority;

  if (vkz_graphics_family_index != vkz_present_family_index) {
    memset(&qci[1], 0, sizeof(VkDeviceQueueCreateInfo));
    qci[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    qci[1].queueFamilyIndex = vkz_graphics_family_index;
    qci[1].queueCount = 1;
    qci[1].pQueuePriorities = &queuePriority;
  }

  VkPhysicalDeviceFeatures pdf;
  memset(&pdf, 0, sizeof(VkPhysicalDeviceFeatures));

  VkDeviceCreateInfo dci;
  memset(&dci, 0, sizeof(VkDeviceCreateInfo));
  dci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  dci.pQueueCreateInfos = qci;
  dci.queueCreateInfoCount = vkz_graphics_family_index ==
    vkz_present_family_index ? 1 : 2;
  dci.pEnabledFeatures = &pdf;

  const char* device_extensions1[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
  const char* device_extensions2[] = { "VK_KHR_portability_subset", VK_KHR_SWAPCHAIN_EXTENSION_NAME };

  if (VK_KHR_portability_subset_supported) {
    LOGDEBUG0("Enabling VK_KHR_portability_subset");
    dci.ppEnabledExtensionNames = (const char* const*)device_extensions2;
    dci.enabledExtensionCount = 3;
  }
  else {
    dci.ppEnabledExtensionNames = (const char* const*)device_extensions1;
    dci.enabledExtensionCount = 1;
  }

#ifndef NDEBUG

  if (validation_layer_count > 0) {
    dci.enabledLayerCount = validation_layer_count;
    dci.ppEnabledLayerNames = validation_layers;
  }
  else {
    dci.enabledLayerCount = 0;
  }
#else
  dci.enabledLayerCount = 0;
#endif

  LOGDEBUG0("Creating logical device...");

  logical_device_created =
    vkCreateDevice(vkz_physical_device, &dci, NULL, &vkz_logical_device) ==
    VK_SUCCESS;

  if (logical_device_created) {
    LOGDEBUG0("Logical device created.");
    vkGetDeviceQueue(vkz_logical_device, vkz_graphics_family_index, 0,
      &vkz_graphics_queue);
    LOGDEBUG0("Graphics queue retrieved.");
    vkGetDeviceQueue(vkz_logical_device, vkz_present_family_index, 0,
      &vkz_present_queue);
    LOGDEBUG0("Present queue retrieved.");
  }
  else {
    LOGDEBUG0("Failed to create logical device!");
  }

  free(qci);

  return logical_device_created;
}

int create_depth_image(void) {

  VkFormat candidate_formats[3];
  candidate_formats[0] = VK_FORMAT_D32_SFLOAT;
  candidate_formats[1] = VK_FORMAT_D32_SFLOAT_S8_UINT;
  candidate_formats[2] = VK_FORMAT_D24_UNORM_S8_UINT;

  BOOL found = FALSE;
  for (int i = 0; i < 3; ++i) {
    depth_image_format = candidate_formats[i];
    VkFormatProperties fp;
    vkGetPhysicalDeviceFormatProperties(vkz_physical_device, depth_image_format, &fp);

    if (fp.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
      found = TRUE;
      break;
    }
  }

  if (!found) {
    LOGDEBUG0("Could not find appropriate format for depth image!");
    return 0;
  }

  if (!vkz_create_image(&depth_image, vkz_width, vkz_height,
    depth_image_format,
    VK_IMAGE_TILING_OPTIMAL,
    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
    &depth_image_memory,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
    return 0;
  }

  if (!vkz_create_image_view(&depth_image_view, depth_image,
    depth_image_format, VK_IMAGE_ASPECT_DEPTH_BIT)) {
    return 0;
  }

  return vkz_transition_image_layout(depth_image, depth_image_format,
    VK_IMAGE_LAYOUT_UNDEFINED,
    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

int destroy_depth_image(void) {
  vkDestroyImageView(vkz_logical_device, depth_image_view, NULL);
  vkDestroyImage(vkz_logical_device, depth_image, NULL);
  vkFreeMemory(vkz_logical_device, depth_image_memory, NULL);
  return 1;
}

int vkz_init(uint32_t max_frames) {

  max_frames_prepared = max_frames;

  vkz_clear_colour.float32[0] = 0.0f;
  vkz_clear_colour.float32[1] = 0.0f;
  vkz_clear_colour.float32[2] = 0.0f;
  vkz_clear_colour.float32[3] = 1.0f;

  if (!(select_physical_device() && select_queue_families() &&
    create_logical_device())) {
    return 0;
  }

  VkCommandPoolCreateInfo command_pool_ci;
  memset(&command_pool_ci, 0, sizeof(VkCommandPoolCreateInfo));
  command_pool_ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  command_pool_ci.queueFamilyIndex = vkz_graphics_family_index;
  command_pool_ci.flags = 0;

  if (vkCreateCommandPool(vkz_logical_device, &command_pool_ci, NULL,
    &command_pool) != VK_SUCCESS) {
    LOGDEBUG0("Could not create command pool!");
    return 0;
  }

  LOGDEBUG0("Command pool created.");

  return 1;
}

int select_swap_extent() {

  vkz_swap_extent.width = vkz_width;
  vkz_swap_extent.height = vkz_height;

  return 1;
}

int vkz_create_image_view(VkImageView* image_view, VkImage image,
  VkFormat format, VkImageAspectFlags aspect_flags) {
  VkImageViewCreateInfo ci;
  memset(&ci, 0, sizeof(VkImageViewCreateInfo));
  ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  ci.image = image;
  ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
  ci.format = format;
  ci.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  ci.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  ci.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  ci.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
  ci.subresourceRange.aspectMask = aspect_flags;
  ci.subresourceRange.baseMipLevel = 0;
  ci.subresourceRange.levelCount = 1;
  ci.subresourceRange.baseArrayLayer = 0;
  ci.subresourceRange.layerCount = 1;

  if (vkCreateImageView(vkz_logical_device, &ci, NULL,
    image_view) != VK_SUCCESS) {
    LOGDEBUG0("Failed to create image view!");
    return 0;
  }

  return 1;
}

int create_swapchain_image_views() {

  vkz_swapchain_image_views = malloc(vkz_swapchain_image_count *
    sizeof(VkImageView));

  if (!vkz_swapchain_image_views) return 0;

  memset(vkz_swapchain_image_views, 0, vkz_swapchain_image_count *
    sizeof(VkImageView));

  for (uint32_t n = 0; n < vkz_swapchain_image_count; n++) {
    if (!vkz_create_image_view(&vkz_swapchain_image_views[n],
      vkz_swapchain_images[n],
      vkz_surface_format.format,
      VK_IMAGE_ASPECT_COLOR_BIT)) {
      return 0;
    }
  }

  swapchain_image_views_created = TRUE;

  return 1;
}

int create_render_pass() {
  VkAttachmentDescription color_buffer_attachment_description;
  memset(&color_buffer_attachment_description, 0,
    sizeof(VkAttachmentDescription));
  color_buffer_attachment_description.format = vkz_surface_format.format;
  color_buffer_attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
  color_buffer_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color_buffer_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  color_buffer_attachment_description.stencilLoadOp =
    VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color_buffer_attachment_description.stencilStoreOp =
    VK_ATTACHMENT_STORE_OP_DONT_CARE;
  color_buffer_attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  color_buffer_attachment_description.finalLayout =
    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference color_attachment_reference;
  memset(&color_attachment_reference, 0, sizeof(VkAttachmentReference));
  color_attachment_reference.attachment = 0;
  color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentDescription depth_attachment_description;
  memset(&depth_attachment_description, 0, sizeof(VkAttachmentDescription));

  depth_attachment_description.format = depth_image_format;

  depth_attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
  depth_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depth_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  depth_attachment_description.stencilLoadOp =
    VK_ATTACHMENT_LOAD_OP_LOAD;
  depth_attachment_description.stencilStoreOp =
    VK_ATTACHMENT_STORE_OP_STORE;
  depth_attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depth_attachment_description.finalLayout =
    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depth_attachment_reference;
  memset(&depth_attachment_reference, 0, sizeof(VkAttachmentReference));
  depth_attachment_reference.attachment = 1;
  depth_attachment_reference.layout =
    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass_description;
  memset(&subpass_description, 0, sizeof(VkSubpassDescription));
  subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass_description.colorAttachmentCount = 1;
  subpass_description.pColorAttachments = &color_attachment_reference;
  subpass_description.pDepthStencilAttachment = &depth_attachment_reference;

  VkSubpassDependency sd;
  memset(&sd, 0, sizeof(VkSubpassDependency));
  sd.srcSubpass = VK_SUBPASS_EXTERNAL;
  sd.dstSubpass = 0;
  sd.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  sd.srcAccessMask = 0;
  sd.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  sd.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  VkAttachmentDescription attachments[2];
  attachments[0] = color_buffer_attachment_description;
  attachments[1] = depth_attachment_description;

  VkRenderPassCreateInfo render_pass_ci;
  memset(&render_pass_ci, 0, sizeof(VkRenderPassCreateInfo));
  render_pass_ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  render_pass_ci.attachmentCount = 2;
  render_pass_ci.pAttachments = attachments;
  render_pass_ci.subpassCount = 1;
  render_pass_ci.pSubpasses = &subpass_description;
  render_pass_ci.dependencyCount = 1;
  render_pass_ci.pDependencies = &sd;

  if (vkCreateRenderPass(vkz_logical_device, &render_pass_ci, NULL,
    &render_pass) != VK_SUCCESS) {
    LOGDEBUG0("Render pass creation failed!");
    return 0;
  }
  else {
    LOGDEBUG0("Render pass created.");
  }
  return 1;
}

int create_framebuffers(VkRenderPass render_pass) {
  framebuffers = malloc(vkz_swapchain_image_count *
    sizeof(VkFramebuffer));

  for (uint32_t n = 0; n < vkz_swapchain_image_count; n++) {

    VkImageView attachments[2];
    attachments[0] = vkz_swapchain_image_views[n];
    attachments[1] = depth_image_view;

    VkFramebufferCreateInfo framebuffer_ci;
    memset(&framebuffer_ci, 0, sizeof(VkFramebufferCreateInfo));
    framebuffer_ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_ci.renderPass = render_pass;
    framebuffer_ci.attachmentCount = 2;
    framebuffer_ci.pAttachments = attachments;
    framebuffer_ci.width = vkz_width;
    framebuffer_ci.height = vkz_height;
    framebuffer_ci.layers = 1;

    if (!framebuffers) return 0;

    if (vkCreateFramebuffer(vkz_logical_device, &framebuffer_ci, NULL,
      &framebuffers[n]) != VK_SUCCESS) {
      LOGDEBUG0("Could not create framebuffers!");

      free(framebuffers);
      framebuffers = NULL;
      return 0;
    }
  }
  return 1;
}

int vkz_set_width_height(const uint32_t width, const uint32_t height) {
  vkz_width = width;
  vkz_height = height;
  return 1;
}

int vkz_create_swapchain() {
  select_swap_extent();

  uint32_t ic = vkz_swapchain_support_details.capabilities.minImageCount;
  if (vkz_swapchain_support_details.capabilities.maxImageCount >= 3) {
    ic = 3;
  }
  else if (vkz_swapchain_support_details.capabilities.maxImageCount >= 2) {
    ic = 2;
  }

  LOGDEBUG1("Swapchain image count: %d", ic);

  VkSwapchainCreateInfoKHR ci;
  memset(&ci, 0, sizeof(VkSwapchainCreateInfoKHR));

  ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  ci.surface = vkz_surface;
  ci.minImageCount = ic;
  ci.imageFormat = vkz_surface_format.format;
  ci.imageColorSpace = vkz_surface_format.colorSpace;
  ci.imageExtent = vkz_swap_extent;
  ci.imageArrayLayers = 1;
  ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

  if (vkz_graphics_family_index != vkz_present_family_index) {
    ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    ci.queueFamilyIndexCount = 2;

    uint32_t ids[2];
    ids[0] = vkz_graphics_family_index;
    ids[1] = vkz_present_family_index;
    ci.pQueueFamilyIndices = ids;

  }
  else {
    ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.queueFamilyIndexCount = 0;
    ci.pQueueFamilyIndices = NULL;
  }

#ifdef __ANDROID__
  ci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
#else
  ci.preTransform = vkz_swapchain_support_details.capabilities.currentTransform;
#endif
  const VkCompositeAlphaFlagBitsKHR af[4] = {
    VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
    VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
    VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
};

  for (uint32_t i = 0; i < sizeof(af); i++) {
    if (vkz_swapchain_support_details.capabilities.supportedCompositeAlpha &
      af[i]) {
      ci.compositeAlpha = af[i];
      break;
    }
  }

  ci.presentMode = vkz_present_mode;

  ci.clipped = VK_TRUE;

  ci.oldSwapchain = VK_NULL_HANDLE;

  if (vkCreateSwapchainKHR(vkz_logical_device, &ci, NULL, &vkz_swapchain) !=
    VK_SUCCESS) {
    return 0;
  }
  else {
    swapchain_created = TRUE;
  }

  vkGetSwapchainImagesKHR(vkz_logical_device, vkz_swapchain,
    &vkz_swapchain_image_count, NULL);
  LOGDEBUG1("Number of presentable images in swapchain: %d",
    vkz_swapchain_image_count);
  vkz_swapchain_images = malloc(vkz_swapchain_image_count * sizeof(VkImage));
  vkGetSwapchainImagesKHR(vkz_logical_device, vkz_swapchain,
    &vkz_swapchain_image_count,
    vkz_swapchain_images);

  return create_swapchain_image_views() && create_depth_image() &&
    create_render_pass() && create_framebuffers(render_pass);
}

int vkz_destroy_swapchain(void) {

  vkDeviceWaitIdle(vkz_logical_device);

  for (uint32_t n = 0; n < vkz_swapchain_image_count; n++) {
    vkDestroyFramebuffer(vkz_logical_device,
      framebuffers[n], NULL);
  }
  free(framebuffers);

  destroy_depth_image();

  vkDestroyRenderPass(vkz_logical_device,
    render_pass, NULL);

  if (vkz_swapchain_image_views) {
    if (swapchain_image_views_created) {
      for (uint32_t n = 0; n < vkz_swapchain_image_count; n++) {
        vkDestroyImageView(vkz_logical_device, vkz_swapchain_image_views[n],
          NULL);
      }
    }
    free(vkz_swapchain_image_views);
    swapchain_image_views_created = FALSE;
  }

  vkz_swapchain_images = NULL;

  if (swapchain_created) {
    vkDestroySwapchainKHR(vkz_logical_device, vkz_swapchain, NULL);
    swapchain_created = FALSE;
    vkz_swapchain = VK_NULL_HANDLE;

  }
  return 1;
}

long alloc_load_shader_spv(char* path, uint32_t** spv) {
  long fs = 0;
  LOGDEBUG1("About to load shader from %s", path);
#ifdef __ANDROID__
  AAsset* asset = AAssetManager_open(vkz_android_app->activity->assetManager,
    path, AASSET_MODE_STREAMING);
  if (!asset) {
    LOGDEBUG1("Could not open file %s!", path);
    return 0;
  }
  fs = AAsset_getLength(asset);
  const void* buffer = AAsset_getBuffer(asset);
  AAsset_close(asset);
  *spv = (uint32_t*)buffer;
#else
  FILE* f = 0;
  f = fopen(path, "rb");
  if (f) {
    fseek(f, 0, SEEK_END);
    fs = ftell(f);
    fseek(f, 0, SEEK_SET);
    unsigned char* spvint = NULL;
    spvint = malloc(fs);
    if (!spvint) {
      LOGDEBUG1("Could not allocate memory to read spv %s!", path);
      return 0;
    }
    fread(spvint, 1, fs, f);
    fclose(f);
    *spv = (uint32_t*)spvint;
  }
  else {
    LOGDEBUG1("Could not open file %s!", path);
    return 0;
  }
#endif

  return fs;
}

int vkz_create_pipeline(const char* vertex_shader_path, const char* fragment_shader_path,
  int (*set_input_state)(VkPipelineVertexInputStateCreateInfo*),
  int (*set_pipeline_layout)(VkPipelineLayoutCreateInfo*), uint32_t* index) {

  if (pipeline_systems) {
    if (pipeline_system_count) {
      if (*index == 100) {
        // Create pipeline in new slot
        pipeline_system_struct* temp =
          malloc((pipeline_system_count + (size_t)1) * sizeof(pipeline_system_struct));
        for (uint32_t n = 0; n < pipeline_system_count; n++) {
          temp[n] = pipeline_systems[n];
        }
        *index = pipeline_system_count;
        free(pipeline_systems);
        pipeline_systems = temp;

        // Create a pipeline layout with the same index

        VkPipelineLayout* ptemp =
          malloc((pipeline_system_count + (size_t)1) * sizeof(VkPipelineLayout));
        for (uint32_t n = 0; n < pipeline_system_count; n++) {
          ptemp[n] = vkz_pipeline_layout[n];
        }
        free(vkz_pipeline_layout);
        vkz_pipeline_layout = ptemp;

        pipeline_system_count++;
        memset(&pipeline_systems[*index], 0, sizeof(pipeline_system_struct));
        size_t path_length = strchr(vertex_shader_path, '\0') -
          vertex_shader_path + 1;
        pipeline_systems[*index].vertex_shader_path = malloc(path_length);
        strcpy(pipeline_systems[*index].vertex_shader_path, (char*)vertex_shader_path);
        path_length = strchr(fragment_shader_path, '\0') - fragment_shader_path + 1;
        pipeline_systems[*index].fragment_shader_path = malloc(path_length);
        strcpy(pipeline_systems[*index].fragment_shader_path, (char*)fragment_shader_path);
        pipeline_systems[*index].set_input_state_function = set_input_state;
        pipeline_systems[*index].set_pipeline_layout_function =
          set_pipeline_layout;
        pipeline_systems[*index].deleted = FALSE;
      }
      else {
        // Reuse existing slot

        if (*index >= pipeline_system_count) {
          LOGDEBUG1("Pipeline %d has ever been created! Cannot reuse index!",
            *index);
          return 0;
        }

        if (pipeline_systems[*index].deleted == FALSE) {
          LOGDEBUG1("Pipeline %d has not been deleted! Cannot reuse index!",
            *index);
          return 0;
        }

        char* vsp = pipeline_systems[*index].vertex_shader_path;
        char* fsp = pipeline_systems[*index].fragment_shader_path;
        int (*sis)(VkPipelineVertexInputStateCreateInfo*) =
          pipeline_systems[*index].set_input_state_function;
        int (*spl)(VkPipelineLayoutCreateInfo*) =
          pipeline_systems[*index].set_pipeline_layout_function;
        memset(&pipeline_systems[*index], 0, sizeof(pipeline_system_struct));
        if (!vsp && vertex_shader_path) {
          size_t path_length = strchr(vertex_shader_path, '\0') - vertex_shader_path + 1;
          pipeline_systems[*index].vertex_shader_path = malloc(path_length);
          strcpy(pipeline_systems[*index].vertex_shader_path, (char*)vertex_shader_path);
        }
        else {
          pipeline_systems[*index].vertex_shader_path = vsp;
        }
        if (!fsp && vertex_shader_path) {
          size_t path_length = strchr(fragment_shader_path, '\0') - fragment_shader_path + 1;
          pipeline_systems[*index].fragment_shader_path = malloc(path_length);
          strcpy(pipeline_systems[*index].fragment_shader_path, (char*)fragment_shader_path);
        }
        else {
          pipeline_systems[*index].fragment_shader_path = fsp;
        }

        pipeline_systems[*index].set_input_state_function = sis;
        pipeline_systems[*index].set_pipeline_layout_function = spl;
        pipeline_systems[*index].deleted = FALSE;
      }
    }
    else {
      LOGDEBUG0("Inconsistent pipeline system state!");
      return 0;
    }
  }
  else {
    // Pipeline has never been created
    if (*index != 100) {
      LOGDEBUG1("No pipeline has ever been created! Cannot reuse index %d!",
        *index);
      return 0;
    }
    pipeline_system_count = 1;
    pipeline_systems = malloc(sizeof(pipeline_system_struct));
    memset(pipeline_systems, 0, sizeof(pipeline_system_struct));
    vkz_pipeline_layout = malloc(sizeof(VkPipelineLayout));
    memset(vkz_pipeline_layout, 0, sizeof(VkPipelineLayout));
    *index = 0;

    size_t path_length = strchr(vertex_shader_path, '\0') -
      vertex_shader_path + 1;
    pipeline_systems[*index].vertex_shader_path = malloc(path_length);
    strcpy(pipeline_systems[*index].vertex_shader_path, (char*)vertex_shader_path);
    path_length = strchr(fragment_shader_path, '\0') - fragment_shader_path + 1;
    pipeline_systems[*index].fragment_shader_path = malloc(path_length);


    if (!pipeline_systems[*index].fragment_shader_path) return 0;
    strcpy(pipeline_systems[*index].fragment_shader_path, (char*)fragment_shader_path);

    pipeline_systems[*index].set_input_state_function = set_input_state;
    pipeline_systems[*index].set_pipeline_layout_function = set_pipeline_layout;
    pipeline_systems[*index].deleted = FALSE;
  }

  VkShaderModuleCreateInfo ci;
  memset(&ci, 0, sizeof(VkShaderModuleCreateInfo));
  ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

  uint32_t* vertexShader = NULL;
  if ((ci.codeSize =
    alloc_load_shader_spv(pipeline_systems[*index].vertex_shader_path,
      &vertexShader))) {
    ci.pCode = vertexShader;
    if (vkCreateShaderModule(vkz_logical_device, &ci, NULL,
      &pipeline_systems[*index].vertex_shader_module) !=
      VK_SUCCESS) {
      return 0;
    }
  }
  else {
    return 0;
  }

  uint32_t* fragmentShader = NULL;
  if ((ci.codeSize =
    alloc_load_shader_spv(pipeline_systems[*index].fragment_shader_path,
      &fragmentShader))) {
    ci.pCode = fragmentShader;
    if (vkCreateShaderModule(vkz_logical_device, &ci, NULL,
      &pipeline_systems[*index].fragment_shader_module) !=
      VK_SUCCESS) {
      return 0;
    }
  }
  else {
    return 0;
  }

  VkPipelineShaderStageCreateInfo vertex_shader_stage_ci;
  memset(&vertex_shader_stage_ci, 0, sizeof(VkPipelineShaderStageCreateInfo));
  vertex_shader_stage_ci.sType =
    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertex_shader_stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertex_shader_stage_ci.module = pipeline_systems[*index].vertex_shader_module;
  vertex_shader_stage_ci.pName = "main";

  VkPipelineShaderStageCreateInfo fragment_shader_stage_ci;
  memset(&fragment_shader_stage_ci, 0, sizeof(VkPipelineShaderStageCreateInfo));
  fragment_shader_stage_ci.sType =
    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragment_shader_stage_ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragment_shader_stage_ci.module =
    pipeline_systems[*index].fragment_shader_module;
  fragment_shader_stage_ci.pName = "main";

  VkPipelineShaderStageCreateInfo shaderStages[] = { vertex_shader_stage_ci,
                 fragment_shader_stage_ci };

  VkPipelineVertexInputStateCreateInfo vertex_input_state_ci;
  memset(&vertex_input_state_ci, 0,
    sizeof(VkPipelineVertexInputStateCreateInfo));
  vertex_input_state_ci.sType =
    VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertex_input_state_ci.vertexBindingDescriptionCount = 0;
  vertex_input_state_ci.pVertexBindingDescriptions = NULL;
  vertex_input_state_ci.vertexAttributeDescriptionCount = 0;
  vertex_input_state_ci.pVertexAttributeDescriptions = NULL;

  if (pipeline_systems[*index].set_input_state_function) {
    pipeline_systems[*index].set_input_state_function(&vertex_input_state_ci);
  }

  VkPipelineInputAssemblyStateCreateInfo input_assembly_state_ci;
  memset(&input_assembly_state_ci, 0,
    sizeof(VkPipelineInputAssemblyStateCreateInfo));
  input_assembly_state_ci.sType =
    VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  input_assembly_state_ci.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  input_assembly_state_ci.primitiveRestartEnable = VK_FALSE;

  VkViewport viewport;
  memset(&viewport, 0, sizeof(viewport));
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)vkz_width;
  viewport.height = (float)vkz_height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor;
  memset(&scissor, 0, sizeof(VkRect2D));
  scissor.offset.x = 0;
  scissor.offset.y = 0;
  scissor.extent = vkz_swap_extent;

  VkPipelineViewportStateCreateInfo viewport_state_ci;
  memset(&viewport_state_ci, 0, sizeof(VkPipelineViewportStateCreateInfo));
  viewport_state_ci.sType =
    VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_state_ci.viewportCount = 1;
  viewport_state_ci.pViewports = &viewport;
  viewport_state_ci.scissorCount = 1;
  viewport_state_ci.pScissors = &scissor;

  VkPipelineRasterizationStateCreateInfo rasterization_state_ci;
  memset(&rasterization_state_ci, 0,
    sizeof(VkPipelineRasterizationStateCreateInfo));
  rasterization_state_ci.sType =
    VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterization_state_ci.depthClampEnable = VK_FALSE;
  rasterization_state_ci.rasterizerDiscardEnable = VK_FALSE;
  rasterization_state_ci.polygonMode = VK_POLYGON_MODE_FILL;
  rasterization_state_ci.lineWidth = 1.0f;
  rasterization_state_ci.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterization_state_ci.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterization_state_ci.depthBiasEnable = VK_FALSE;
  rasterization_state_ci.depthBiasConstantFactor = 0.0f;
  rasterization_state_ci.depthBiasClamp = 0.0f;
  rasterization_state_ci.depthBiasSlopeFactor = 0.0f;

  VkPipelineMultisampleStateCreateInfo multisample_state_ci;
  memset(&multisample_state_ci, 0,
    sizeof(VkPipelineMultisampleStateCreateInfo));
  multisample_state_ci.sType =
    VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisample_state_ci.sampleShadingEnable = VK_FALSE;
  multisample_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisample_state_ci.minSampleShading = 1.0f;
  multisample_state_ci.pSampleMask = NULL;
  multisample_state_ci.alphaToCoverageEnable = VK_FALSE;
  multisample_state_ci.alphaToOneEnable = VK_FALSE;

  VkPipelineColorBlendAttachmentState color_blend_attachment_state;
  memset(&color_blend_attachment_state, 0,
    sizeof(VkPipelineColorBlendAttachmentState));
  color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
    VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
    VK_COLOR_COMPONENT_A_BIT;
  color_blend_attachment_state.blendEnable = VK_TRUE;
  color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  color_blend_attachment_state.dstColorBlendFactor =
    VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
  color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;

  VkPipelineColorBlendStateCreateInfo color_blend_state_ci;
  memset(&color_blend_state_ci, 0, sizeof(VkPipelineColorBlendStateCreateInfo));
  color_blend_state_ci.sType =
    VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  color_blend_state_ci.logicOpEnable = VK_FALSE;
  color_blend_state_ci.logicOp = VK_LOGIC_OP_COPY;
  color_blend_state_ci.attachmentCount = 1;
  color_blend_state_ci.pAttachments = &color_blend_attachment_state;
  color_blend_state_ci.blendConstants[0] = 0.0f;
  color_blend_state_ci.blendConstants[1] = 0.0f;
  color_blend_state_ci.blendConstants[2] = 0.0f;
  color_blend_state_ci.blendConstants[3] = 0.0f;

  VkPipelineLayoutCreateInfo pipeline_layout_ci;
  memset(&pipeline_layout_ci, 0, sizeof(VkPipelineLayoutCreateInfo));
  pipeline_layout_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_ci.setLayoutCount = 0;
  pipeline_layout_ci.pSetLayouts = NULL;
  pipeline_layout_ci.pushConstantRangeCount = 0;
  pipeline_layout_ci.pPushConstantRanges = NULL;

  if (pipeline_systems[*index].set_pipeline_layout_function) {
    if (!pipeline_systems[*index].set_pipeline_layout_function(&pipeline_layout_ci)) {
      LOGDEBUG0("Failed to set the pipeline_layout via provided set_pipeline_layout "
        "function!");
    }
  }

  if (vkCreatePipelineLayout(vkz_logical_device, &pipeline_layout_ci, NULL,
    &vkz_pipeline_layout[*index])
    != VK_SUCCESS) {
    LOGDEBUG0("Failed to create pipeline layout!");
  }

  VkPipelineDepthStencilStateCreateInfo depth_stencil_ci;
  memset(&depth_stencil_ci, 0, sizeof(VkPipelineDepthStencilStateCreateInfo));
  depth_stencil_ci.sType =
    VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depth_stencil_ci.depthTestEnable = VK_TRUE;
  depth_stencil_ci.depthWriteEnable = VK_TRUE;
  depth_stencil_ci.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
  depth_stencil_ci.depthBoundsTestEnable = VK_FALSE;
  depth_stencil_ci.minDepthBounds = 0.0f;
  depth_stencil_ci.maxDepthBounds = 1.0f;
  depth_stencil_ci.stencilTestEnable = VK_FALSE;

  VkGraphicsPipelineCreateInfo pipeline_ci;
  memset(&pipeline_ci, 0, sizeof(VkGraphicsPipelineCreateInfo));
  pipeline_ci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_ci.stageCount = 2;
  pipeline_ci.pStages = shaderStages;
  pipeline_ci.pVertexInputState = &vertex_input_state_ci;
  pipeline_ci.pInputAssemblyState = &input_assembly_state_ci;
  pipeline_ci.pViewportState = &viewport_state_ci;
  pipeline_ci.pRasterizationState = &rasterization_state_ci;
  pipeline_ci.pMultisampleState = &multisample_state_ci;
  pipeline_ci.pDepthStencilState = NULL;
  pipeline_ci.pColorBlendState = &color_blend_state_ci;
  pipeline_ci.pDynamicState = NULL;
  pipeline_ci.layout = vkz_pipeline_layout[*index];

  pipeline_ci.renderPass = render_pass;
  pipeline_ci.subpass = 0;

  pipeline_ci.basePipelineHandle = VK_NULL_HANDLE;
  pipeline_ci.basePipelineIndex = -1;

  pipeline_ci.pDepthStencilState = &depth_stencil_ci;

  if (vkCreateGraphicsPipelines(vkz_logical_device, VK_NULL_HANDLE, 1,
    &pipeline_ci, NULL,
    &pipeline_systems[*index].pipeline) !=
    VK_SUCCESS) {
    LOGDEBUG0("Could not create graphics pipeline!");
  }
  else {
    LOGDEBUG0("Pipeline created ok.");
  }

  // Freeing these when loaded as Android assets makes
  // the app crash.
#ifndef __ANDROID__
  if (vertexShader) {
    free(vertexShader);
  }
  if (fragmentShader) {
    free(fragmentShader);
  }
#endif

  return 1;
}

int destroy_pipeline(uint32_t index, BOOL free_shader_path_strings) {

  if (pipeline_system_count <= index) {
    return 0;
  }

  LOGDEBUG1("Destroying pipeline %d.", index);

  vkDestroyPipeline(vkz_logical_device,
    pipeline_systems[index].pipeline, NULL);

  vkDestroyPipelineLayout(vkz_logical_device,
    vkz_pipeline_layout[index], NULL);
  vkDestroyShaderModule(vkz_logical_device,
    pipeline_systems[index].vertex_shader_module, NULL);
  vkDestroyShaderModule(vkz_logical_device,
    pipeline_systems[index].fragment_shader_module, NULL);

  if (free_shader_path_strings) {
    free(pipeline_systems[index].vertex_shader_path);
    free(pipeline_systems[index].fragment_shader_path);
    pipeline_systems[index].vertex_shader_path = NULL;
    pipeline_systems[index].fragment_shader_path = NULL;
  }

  pipeline_systems[index].deleted = TRUE;

  return 1;
}

int vkz_destroy_pipeline(uint32_t index) {
  if (pipeline_systems[index].deleted) return 1;
  return destroy_pipeline(index, FALSE);
}

int vkz_begin_draw_command_buffer(VkCommandBuffer* command_buffer) {

  VkCommandBufferAllocateInfo command_buffer_ai;
  memset(&command_buffer_ai, 0, sizeof(VkCommandBufferAllocateInfo));
  command_buffer_ai.sType =
    VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  command_buffer_ai.commandPool = command_pool;
  command_buffer_ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  command_buffer_ai.commandBufferCount = 1;

  if (vkAllocateCommandBuffers(vkz_logical_device, &command_buffer_ai,
    command_buffer) != VK_SUCCESS) {
    LOGDEBUG0("Could not allocate command buffer.");
    return 0;
  }
  else {
    VkCommandBufferBeginInfo command_buffer_bi;
    memset(&command_buffer_bi, 0, sizeof(VkCommandBufferBeginInfo));
    command_buffer_bi.sType =
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_bi.flags =
      VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    command_buffer_bi.pInheritanceInfo = NULL;

    if (vkBeginCommandBuffer(*command_buffer, &command_buffer_bi) != VK_SUCCESS) {
      LOGDEBUG0("Could not begin recording command buffer!");
      return 0;
    }
    else {

      VkRenderPassBeginInfo render_pass_bi;
      memset(&render_pass_bi, 0, sizeof(VkRenderPassBeginInfo));
      render_pass_bi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
      render_pass_bi.renderPass = render_pass;
      render_pass_bi.framebuffer = framebuffers[next_image_index];
      render_pass_bi.renderArea.offset.x = 0;
      render_pass_bi.renderArea.offset.y = 0;
      render_pass_bi.renderArea.extent = vkz_swap_extent;

      VkClearValue clear_values[2];
      memset(clear_values, 0, 2 * sizeof(VkClearValue));
      clear_values[0].color = vkz_clear_colour;
      clear_values[1].depthStencil.depth = 1.0f;
      clear_values[1].depthStencil.stencil = 0;

      render_pass_bi.clearValueCount = 2;
      render_pass_bi.pClearValues = clear_values;

      vkCmdBeginRenderPass(*command_buffer, &render_pass_bi,
        VK_SUBPASS_CONTENTS_INLINE);

      return 1;

    }
  }
  return 0;
}

int vkz_bind_pipeline_to_command_buffer(uint32_t pipeline_index,
  const VkCommandBuffer* command_buffer) {

  vkCmdBindPipeline(*command_buffer,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    pipeline_systems[pipeline_index].pipeline);
  return 1;

}

int vkz_end_draw_command_buffer(VkCommandBuffer* command_buffer) {
  vkCmdEndRenderPass(*command_buffer);

  if (vkEndCommandBuffer(*command_buffer) != VK_SUCCESS) {
    LOGDEBUG0("Could not end command buffer!");
    return 0;
  }

  return 1;
}

int vkz_destroy_draw_command_buffer(VkCommandBuffer* command_buffer) {

  vkFreeCommandBuffers(vkz_logical_device, command_pool, 1, command_buffer);

  return 1;
}

int vkz_create_sync_objects(void) {

  gpu_cpu_fence = malloc(max_frames_prepared * sizeof(VkFence));
  acquire_semaphore = malloc(max_frames_prepared * sizeof(VkSemaphore));
  draw_semaphore = malloc(max_frames_prepared * sizeof(VkSemaphore));

  for (uint32_t idx = 0; idx < max_frames_prepared; ++idx) {
    gpu_cpu_fence[idx] = VK_NULL_HANDLE;
    acquire_semaphore[idx] = VK_NULL_HANDLE;
    draw_semaphore[idx] = VK_NULL_HANDLE;
  }

  VkFenceCreateInfo fence_ci;
  memset(&fence_ci, 0, sizeof(VkFenceCreateInfo));
  fence_ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fence_ci.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  VkSemaphoreCreateInfo sci;
  memset(&sci, 0, sizeof(VkSemaphoreCreateInfo));

  sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  sci.pNext = NULL;
  sci.flags = 0;

  for (uint32_t idx = 0; idx < max_frames_prepared; ++idx) {
    if (vkCreateFence(vkz_logical_device, &fence_ci, NULL,
      &gpu_cpu_fence[idx]) !=
      VK_SUCCESS) {
      LOGDEBUG1("Could not create cpu gpu fence %d !", idx);
      return 0;
    }

    if (vkCreateSemaphore(vkz_logical_device, &sci, NULL,
      &draw_semaphore[idx]) != VK_SUCCESS) {
      LOGDEBUG1("Could not create draw semaphore %d !", idx);
      return 0;
    }

    if (vkCreateSemaphore(vkz_logical_device, &sci, NULL,
      &acquire_semaphore[idx]) != VK_SUCCESS) {
      LOGDEBUG1("Could not create acquire semaphore %d !", idx);
      return 0;
    }
  }

  LOGDEBUG0("Created sync objects.");

  return 1;
}

int vkz_destroy_sync_objects(void) {
  vkDeviceWaitIdle(vkz_logical_device);
  for (uint32_t idx = 0; idx < max_frames_prepared; ++idx) {
    vkDestroyFence(vkz_logical_device,
      gpu_cpu_fence[idx], NULL);
    gpu_cpu_fence[idx] = VK_NULL_HANDLE;
    vkDestroySemaphore(vkz_logical_device,
      draw_semaphore[idx], NULL);
    draw_semaphore[idx] = VK_NULL_HANDLE;
    vkDestroySemaphore(vkz_logical_device,
      acquire_semaphore[idx], NULL);
    acquire_semaphore[idx] = VK_NULL_HANDLE;
  }

  free(gpu_cpu_fence);
  free(acquire_semaphore);
  free(draw_semaphore);


  return 1;
}

int vkz_recreate_pipelines_and_swapchain(void) {
  LOGDEBUG0("Recreating pipelines and swapchain.");

  for (uint32_t i = 0; i < pipeline_system_count; ++i) {
    destroy_pipeline(i, FALSE);
  }

  vkz_destroy_swapchain();
  vkz_create_swapchain();
  for (uint32_t i = 0; i < pipeline_system_count; ++i) {
    vkz_create_pipeline(NULL, NULL, NULL, NULL, &i);
  }

  return 1;
}

int vkz_acquire_next_image(uint32_t pipeline_index, uint32_t* image_index, uint32_t *frame_index_out) {

  frame_index = (frame_index + 1) % max_frames_prepared;
  *frame_index_out = frame_index;

  VkResult r =
    vkAcquireNextImageKHR(vkz_logical_device, vkz_swapchain,
      UINT64_MAX,
      acquire_semaphore[frame_index],
      VK_NULL_HANDLE, &next_image_index);

  if (r == VK_ERROR_OUT_OF_DATE_KHR) {
    vkz_recreate_pipelines_and_swapchain();
    return 1;
  }
  else if (r != VK_SUCCESS && r != VK_SUBOPTIMAL_KHR) {
    LOGDEBUG0("Could not acquire swapchain image!");
    return 0;
  }

  *image_index = next_image_index;

  return 1;
}

int vkz_present_next_image(void) {

  VkPresentInfoKHR pinf;
  memset(&pinf, 0, sizeof(VkPresentInfoKHR));
  pinf.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  VkSwapchainKHR swap_chains[] = { vkz_swapchain };
  pinf.swapchainCount = 1;
  pinf.pSwapchains = swap_chains;
  pinf.pImageIndices = &next_image_index;
  pinf.pResults = NULL;
  pinf.pWaitSemaphores = &draw_semaphore[frame_index];
  pinf.waitSemaphoreCount = 1;

  VkResult r = vkQueuePresentKHR(vkz_present_queue, &pinf);
  if (r == VK_ERROR_OUT_OF_DATE_KHR || r == VK_SUBOPTIMAL_KHR) {
    vkz_recreate_pipelines_and_swapchain();
  }
  else if (r != VK_SUCCESS) {
    LOGDEBUG0("Could not present swapchain image!");
    return 0;
  }

  return 1;
}

int vkz_draw(VkCommandBuffer* command_buffer) {

  VkSubmitInfo si;
  memset(&si, 0, sizeof(VkSubmitInfo));
  si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  si.commandBufferCount = 1;
  si.pCommandBuffers = command_buffer;
  si.pSignalSemaphores = &draw_semaphore[frame_index];
  si.signalSemaphoreCount = 1;
  si.pWaitSemaphores = &acquire_semaphore[frame_index];
  si.waitSemaphoreCount = 1;

  VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
  si.pWaitDstStageMask = wait_stages;

  vkz_wait_gpu_cpu_fence(frame_index);

  vkResetFences(vkz_logical_device, 1,
    &gpu_cpu_fence[frame_index]);

  if (vkQueueSubmit(vkz_graphics_queue, 1, &si,
    gpu_cpu_fence[frame_index]) != VK_SUCCESS) {
    LOGDEBUG0("Could not submit draw command buffer!");
  }

  return 1;
}

int find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags prop_flags,
  uint32_t* mem_type) {
  BOOL found = FALSE;
  VkPhysicalDeviceMemoryProperties mp;
  memset(&mp, 0, sizeof(VkPhysicalDeviceMemoryProperties));
  vkGetPhysicalDeviceMemoryProperties(vkz_physical_device, &mp);
  for (uint32_t i = 0; i < mp.memoryTypeCount; i++) {
    if ((type_filter & (1 << i)) &&
      (mp.memoryTypes[i].propertyFlags & prop_flags) == prop_flags) {
      *mem_type = i;
      found = TRUE;
      break;
    }
  }
  return found;
}

int vkz_create_buffer(VkBuffer* buffer,
  VkBufferUsageFlags buffer_usage_flags,
  uint32_t buffer_size,
  VkDeviceMemory* memory,
  VkMemoryPropertyFlags memory_property_flags) {
  VkBufferCreateInfo ci;
  memset(&ci, 0, sizeof(VkBufferCreateInfo));
  ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  ci.size = buffer_size;
  ci.usage = buffer_usage_flags;
  ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(vkz_logical_device, &ci, NULL, buffer) !=
    VK_SUCCESS) {
    return 0;
  }

  VkMemoryRequirements mr;
  vkGetBufferMemoryRequirements(vkz_logical_device, *buffer, &mr);

  VkMemoryAllocateInfo ai;
  memset(&ai, 0, sizeof(VkMemoryAllocateInfo));
  ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  ai.allocationSize = mr.size;
  uint32_t mem_type = 0;
  if (!find_memory_type(mr.memoryTypeBits,
    memory_property_flags,
    &mem_type)) {
    vkDestroyBuffer(vkz_logical_device, *buffer, NULL);
    return 0;
  }

  ai.memoryTypeIndex = mem_type;
  if (vkAllocateMemory(vkz_logical_device, &ai, NULL, memory) !=
    VK_SUCCESS) {
    vkDestroyBuffer(vkz_logical_device, *buffer, NULL);
    return 0;
  }
  vkBindBufferMemory(vkz_logical_device, *buffer, *memory, 0);
  return 1;
}

int vkz_destroy_buffer(VkBuffer buffer, VkDeviceMemory memory) {
  vkDestroyBuffer(vkz_logical_device, buffer, NULL);
  vkFreeMemory(vkz_logical_device, memory, NULL);
  return 1;
}

int begin_single_time_commands(VkCommandBuffer* command_buffer) {
  VkCommandBufferAllocateInfo ai;
  memset(&ai, 0, sizeof(VkCommandBufferAllocateInfo));
  ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  ai.commandPool = command_pool;
  ai.commandBufferCount = 1;

  vkAllocateCommandBuffers(vkz_logical_device, &ai, command_buffer);

  VkCommandBufferBeginInfo bi;
  memset(&bi, 0, sizeof(VkCommandBufferBeginInfo));
  bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  vkBeginCommandBuffer(*command_buffer, &bi);

  return 1;
}

int end_single_time_commands(VkCommandBuffer command_buffer) {
  vkEndCommandBuffer(command_buffer);

  VkSubmitInfo si;
  memset(&si, 0, sizeof(VkSubmitInfo));
  si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  si.commandBufferCount = 1;
  si.pCommandBuffers = &command_buffer;

  vkResetFences(vkz_logical_device, 1,
    &gpu_cpu_fence[frame_index]);
  vkQueueSubmit(vkz_graphics_queue, 1, &si, gpu_cpu_fence[frame_index]);

  vkz_wait_gpu_cpu_fence(frame_index);

  vkFreeCommandBuffers(vkz_logical_device,
    command_pool,
    1,
    &command_buffer);
  return 1;
}

int vkz_copy_buffer(VkBuffer source, VkBuffer destination, VkDeviceSize size) {
  VkCommandBuffer cb;
  begin_single_time_commands(&cb);

  VkBufferCopy bc;
  memset(&bc, 0, sizeof(VkBufferCopy));
  bc.srcOffset = 0;
  bc.dstOffset = 0;
  bc.size = size;
  vkCmdCopyBuffer(cb, source, destination, 1, &bc);

  return end_single_time_commands(cb);
}

int vkz_create_image(VkImage* image,
  uint32_t image_width, uint32_t image_height,
  VkFormat image_format, VkImageTiling image_tiling,
  VkImageUsageFlags image_usage_flags,
  VkDeviceMemory* memory,
  VkMemoryPropertyFlags memory_property_flags) {
  VkImageCreateInfo ci;
  memset(&ci, 0, sizeof(VkImageCreateInfo));
  ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  ci.imageType = VK_IMAGE_TYPE_2D;
  ci.extent.width = image_width;
  ci.extent.height = image_height;
  ci.extent.depth = 1;
  ci.mipLevels = 1;
  ci.arrayLayers = 1;
  ci.format = image_format;
  ci.tiling = image_tiling;
  ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  ci.usage = image_usage_flags;
  ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  ci.samples = VK_SAMPLE_COUNT_1_BIT;
  ci.flags = 0;

  if (vkCreateImage(vkz_logical_device, &ci, NULL, image) != VK_SUCCESS) {
    return 0;
  }

  VkMemoryRequirements mr;
  vkGetImageMemoryRequirements(vkz_logical_device, *image, &mr);

  VkMemoryAllocateInfo ai;
  memset(&ai, 0, sizeof(VkMemoryAllocateInfo));
  ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  ai.allocationSize = mr.size;
  if (!find_memory_type(mr.memoryTypeBits, memory_property_flags,
    &ai.memoryTypeIndex)) {
    return 0;
  }

  if (vkAllocateMemory(vkz_logical_device, &ai, NULL, memory) !=
    VK_SUCCESS) {
    return 0;
  }

  vkBindImageMemory(vkz_logical_device, *image, *memory, 0);

  return 1;

}

int vkz_destroy_image(VkImage image, VkDeviceMemory image_memory) {

  if (image != VK_NULL_HANDLE) {
    vkDestroyImage(vkz_logical_device, image, NULL);
    image = VK_NULL_HANDLE;
  }

  if (image_memory != VK_NULL_HANDLE) {
    vkFreeMemory(vkz_logical_device, image_memory, NULL);
    image_memory = VK_NULL_HANDLE;
  }

  return 1;
}

int vkz_transition_image_layout(VkImage image, VkFormat format,
  VkImageLayout old_layout,
  VkImageLayout new_layout) {
  VkCommandBuffer cb;
  begin_single_time_commands(&cb);

  VkPipelineStageFlags source_stage, destination_stage;

  VkImageMemoryBarrier mb;
  memset(&mb, 0, sizeof(VkImageMemoryBarrier));
  mb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  mb.oldLayout = old_layout;
  mb.newLayout = new_layout;
  mb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  mb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  mb.image = image;
  if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    mb.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    if (format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
      format == VK_FORMAT_D24_UNORM_S8_UINT) {
      mb.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
  }
  else {
    mb.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  }
  mb.subresourceRange.baseMipLevel = 0;
  mb.subresourceRange.levelCount = 1;
  mb.subresourceRange.baseArrayLayer = 0;
  mb.subresourceRange.layerCount = 1;

  if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout ==
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    mb.srcAccessMask = 0;
    mb.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  }
  else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
    new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    mb.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mb.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  }
  else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
    new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    mb.srcAccessMask = 0;
    mb.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
      VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  }
  else if (old_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR &&
    new_layout == VK_IMAGE_LAYOUT_GENERAL)
  {
    source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destination_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

  }
  else {
    LOGDEBUG0("Unsupported layout transition!");
    return 0;
  }

  vkCmdPipelineBarrier(cb, source_stage, destination_stage, 0, 0, NULL, 0,
    NULL, 1, &mb);

  return end_single_time_commands(cb);
}

int vkz_copy_buffer_to_image(VkBuffer buffer, VkImage image,
  uint32_t width, uint32_t height) {
  VkCommandBuffer cb;
  begin_single_time_commands(&cb);

  VkBufferImageCopy bic;
  memset(&bic, 0, sizeof(VkBufferImageCopy));
  bic.bufferOffset = 0;
  bic.bufferRowLength = 0;
  bic.bufferImageHeight = 0;
  bic.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  bic.imageSubresource.mipLevel = 0;
  bic.imageSubresource.baseArrayLayer = 0;
  bic.imageSubresource.layerCount = 1;
  bic.imageOffset.x = 0;
  bic.imageOffset.y = 0;
  bic.imageOffset.z = 0;
  bic.imageExtent.width = width;
  bic.imageExtent.height = height;
  bic.imageExtent.depth = 1;

  vkCmdCopyBufferToImage(cb, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    1, &bic);

  end_single_time_commands(cb);
  return 1;
}

int vkz_create_sampler(VkSampler* sampler) {
  VkSamplerCreateInfo sci;
  memset(&sci, 0, sizeof(VkSamplerCreateInfo));
  sci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  sci.magFilter = VK_FILTER_LINEAR;
  sci.minFilter = VK_FILTER_LINEAR;
  sci.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sci.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sci.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sci.anisotropyEnable = VK_FALSE; // Was true in the tutorial
  sci.maxAnisotropy = 16;
  sci.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  sci.unnormalizedCoordinates = VK_FALSE;
  sci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  sci.mipLodBias = 0.0f;
  sci.minLod = 0.0f;
  sci.maxLod = 0.0f;
  sci.compareEnable = VK_TRUE;
  if (vkCreateSampler(vkz_logical_device, &sci, NULL, sampler) != VK_SUCCESS) {
    return 0;
  }
  return 1;
}

int vkz_shutdown(void) {

  if (logical_device_created) {
    vkDeviceWaitIdle(vkz_logical_device);
  }

  if (pipeline_systems) {

    for (uint32_t n = 0; n < pipeline_system_count; ++n) {
      if (!pipeline_systems[n].deleted) {
        destroy_pipeline(n, TRUE);
      }
    }

    free(pipeline_systems);
    pipeline_systems = NULL;
  }

  pipeline_system_count = 0;

  if (vkz_pipeline_layout) {
    free(vkz_pipeline_layout);
  }

  if (vkz_swapchain_support_details.formats)
    free(vkz_swapchain_support_details.formats);

  if (vkz_swapchain_support_details.presentModes)
    free(vkz_swapchain_support_details.presentModes);

  if (logical_device_created) {
    LOGDEBUG0("Destroying command pool.");
    vkDestroyCommandPool(vkz_logical_device,
      command_pool, NULL);
    LOGDEBUG0("Destroying logical device.");
    vkDestroyDevice(vkz_logical_device, NULL);
  }

  if (debug_callback_created) {
    LOGDEBUG0("Destroying debug callback.");
    PFN_vkDestroyDebugReportCallbackEXT dcDestroy =
      (PFN_vkDestroyDebugReportCallbackEXT)
      vkGetInstanceProcAddr(vkz_instance, "vkDestroyDebugReportCallbackEXT");
    if (dcDestroy != NULL) {
      dcDestroy(vkz_instance, callback, NULL);
    }
    else {
      LOGDEBUG0("Could not get pointer to debug report callback destructor!");
    }
  }

  if (instance_created) {
    LOGDEBUG0("Destroying instance...");
    vkDestroyInstance(vkz_instance, NULL);
    instance_created = FALSE;
  }
  return 1;
}
