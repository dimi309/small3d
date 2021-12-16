/**
 * @file vkzos.h
 * @brief Vulkan helper functions.
 *
 * Created on: 2018/05/01
 *     Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */
#ifndef VKZOS_H
#define VKZOS_H

#ifdef __ANDROID__
#include <android_native_app_glue.h>
#include <android/asset_manager.h>
#define VK_USE_PLATFORM_ANDROID_KHR
extern struct android_app *vkz_android_app;
#endif

#ifdef SMALL3D_IOS
#include <MoltenVK/mvk_vulkan.h>
#else
#include <vulkan/vulkan.h>
#endif

/**
 * @brief The Vulkan instance
 */
 extern VkInstance vkz_instance;

/**
 * @brief The surface on which graphics will be presented.
 */
extern VkSurfaceKHR vkz_surface;

/**
 * @brief The physical device that is selected vkz_init().
 */

extern VkPhysicalDevice vkz_physical_device;

/**
 * @brief The logical device that is created by vkz_init().
 */
extern VkDevice vkz_logical_device;

/**
 * @brief The pipeline layouts, one for each pipeline created.
 */
extern VkPipelineLayout* vkz_pipeline_layout;

/**
 * @brief Number of images in the swapchain, set by vkz_create_swapchain().
 */
extern uint32_t vkz_swapchain_image_count;

/**
 * @brief The colour used to clear the screen.
 */
extern VkClearColorValue vkz_clear_colour;

/**
 * @brief  Create a vulkan instance
 * @param  application_name The name of the application that will be using Vulkan
 * @param  enabled_extension_names Names of extensions used
 * @param  enabled_extension_count Number of extensions used
 * @return 1 if successful, 0 otherwise
 */
int vkz_create_instance(const char* application_name,
  const char** enabled_extension_names,
  size_t enabled_extension_count);
/**
 * @brief  Initialise. Internally this means create physical device, select queue
 *         families and create logical device.
 * @return 1 if successful, 0 otherwise
 */
int vkz_init(void);

/**
 * @brief Set width and height for all rendering calculations.
 * @param width The new width
 * @param height The new height
 * @return 1 if successful, 0 otherwise
 */
int vkz_set_width_height(const uint32_t width, const uint32_t height);

/**
 * @brief Create the swapchain that will be used. This will also create
 * the associated image views.
 * @return 1 if successful, 0 otherwise
 */
int vkz_create_swapchain();

/**
 * @brief Destroy the swapchain.
 * @return 1 if successful, 0 otherwise
 */
int vkz_destroy_swapchain(void);

/**
 * @brief Create the image used for depth testing. This function must be called
 *        right after vkz_create_swapchain(). 
 * @return 1 if successful, 0 otherwise
 */
int vkz_create_depth_image(void);

/**
 * @brief Destroy the image used for depth testing.
 * @return 1 if successful, 0 otherwise
 */
int vkz_destroy_depth_image(void);

/**
 * @brief Create a pipeline. This will also create associated Vulkan
 *        objects.
 *
 * @param vertex_shader_path   The path to the SPV file for the vertex shader
 *                             that will be used in the pipeline.
 * @param fragment_shader_path The path to the SPV file for the fragment shader
 *                             that will be used in the pipeline.
 * @param set_input_state      Callback function, allowing e.g. the addition of
 *                             information on buffers that will be used by the
 *                             pipeline (vertex, uniform) before the pipeline is
 *                             created. If there is no such information to be added,
 *                             NULL should be passed here.
 * @param set_pipeline_layout  Callback function, allowing e.g. the addition of
 *                             information on descriptors that will be used by the
 *                             pipeline before the pipeline is created. If there is
 *                             no such information to be added, NULL should be passed
 *                             here.
 * @param index If 100 is passed here, a new "slot" will be used for the pipeline
 *              and returned via the same variable. If not, the pipeline will be
 *              created over a previously deleted pipeline at the index position of
 *              the internal array containing pipelines. In that case, the shader
 *              path parameters will be ignored, because the previously used shaders
 *              will be reused. Also in that case the callback functions parameters
 *              will be ignored.
 * @return      1 if successful, 0 otherwise
 */
int vkz_create_pipeline(const char* vertex_shader_path, const char* fragment_shader_path,
  int (*set_input_state)(VkPipelineVertexInputStateCreateInfo*),
  int (*set_pipeline_layout)(VkPipelineLayoutCreateInfo*), uint32_t* index);

/**
 * @brief Destroy a pipeline
 * @param index The index of the pipeline to destroy
 */
int vkz_destroy_pipeline(uint32_t index);

/**
 * @brief Begin creating a drawing command buffer 
 *
 * @param command_buffer Pointer to the command buffer.
 * @return 1 if successful, 0 otherwise
 */
int vkz_begin_draw_command_buffer(VkCommandBuffer* command_buffer);

/**
 * @brief Bind a pipeline to a command buffer
 *
 * @param pipeline_index The index of the pipeline
 * @param command_buffer Pointer to the command buffer.
 * @return 1 if successful, 0 otherwise
 */
int vkz_bind_pipeline_to_command_buffer(uint32_t pipeline_index,
  const VkCommandBuffer* command_buffer);

/**
 * @brief Finish recording a command buffer.
 *
 * @param command_buffer Pointer to the command buffer.
 * @return 1 if successful, 0 otherwise
 */
int vkz_end_draw_command_buffer(VkCommandBuffer* command_buffer);

/**
 * @brief Destroy a command buffer.
 *
 * @param command_buffer Pointer to the command buffer.
 * @return 1 if successful, 0 otherwise
 */
int vkz_destroy_draw_command_buffer(VkCommandBuffer* command_buffer);

/**
 * @brief Create sync objects 
 * @return 1 if successful, 0 otherwise
 */
int vkz_create_sync_objects(void);

/**
 * @brief Destroy sync objects
 * @return 1 if successful, 0 otherwise
 */
int vkz_destroy_sync_objects(void);

/**
 * @brief Recreate the swapchain and the pipelines.
 * @return 1 if successful, 0 otherwise
 */
int vkz_recreate_pipelines_and_swapchain(void);

/**
 * @brief Acquire next swapchain image
 *
 * @param pipeline_index The index of the pipeline
 * @param image_index    The index of the acquired swapchain image
 * @return 1 if successful, 0 otherwise
 */
int vkz_acquire_next_image(uint32_t pipeline_index, uint32_t* image_index);

/**
 * @brief Present next swapchain image (the one acquired by
 *        vkz_acquire_next_image())
 * @return 1 if successful, 0 otherwise
 */
int vkz_present_next_image(void);

/**
 * @brief  Send draw commands (will take effect on the current pipeline image
 *         acquired by vkz_acquire_next_image())
 * @param  command_buffer Pointer to the command buffer containing the commands.
 * @return 1 if successful, 0 otherwise
 */
int vkz_draw(VkCommandBuffer* command_buffer);

/**
 * @brief  Create a buffer
 * @param  buffer                Variable in which to store the buffer
 * @param  buffer_usage_flags    Vulkan usage flags for the buffer
 * @param  buffer_size           The size of the buffer, in bytes
 * @param  memory                The GPU memory reserved for the buffer
 * @param  memory_property_flags Vulkan properties of the reserved memory
 * @return 1 if successful, 0 otherwise
 */
int vkz_create_buffer(VkBuffer* buffer,
  VkBufferUsageFlags buffer_usage_flags,
  uint32_t buffer_size,
  VkDeviceMemory* memory,
  VkMemoryPropertyFlags memory_property_flags);

/**
 * @brief  Destroy a buffer
 * @param  buffer The buffer to be destroyed
 * @param  memory The GPU memory previously reserved for the buffer,
 *               to be released
 * @return 1 if successful, 0 otherwise
 */
int vkz_destroy_buffer(VkBuffer buffer, VkDeviceMemory memory);

/**
 * @brief Copy a buffer to another
 * @param source         The source buffer
 * @param destination    The destination buffer
 * @param size           The size of the buffer to be copied, in bytes
 * @return 1 if successful, 0 otherwise
 */
int vkz_copy_buffer(VkBuffer source, VkBuffer destination, VkDeviceSize size);

/**
 * @brief Create a Vulkan image
 * @param image Variable in which to store the image
 * @param image_width           The width of the image
 * @param image_height          The height of the image
 * @param image_format          The format of the image
 * @param image_tiling          The tiling of the image
 * @param image_usage_flags     The usage flags of the image
 * @param memory                The GPU memory reserved for the image
 * @param memory_property_flags Vulkan properties of the reserved memory
 * @return 1 if successful, 0 otherwise
 */
int vkz_create_image(VkImage* image,
  uint32_t image_width, uint32_t image_height,
  VkFormat image_format, VkImageTiling image_tiling,
  VkImageUsageFlags image_usage_flags,
  VkDeviceMemory* memory,
  VkMemoryPropertyFlags memory_property_flags);

/**
 * @brief Destroy a Vulkan image
 * @param image        The image to be destroyed
 * @param image_memory The GPU memory previously reserved for the image,
 *                     to be released
 * @return 1 if successful, 0 otherwise
 */
int vkz_destroy_image(VkImage image, VkDeviceMemory image_memory);

/**
 * @brief Transition the layout of a Vulkan image
 * @param image          The Vulkan image
 * @param format         The image format
 * @param old_layout     The old layout
 * @param new_layout     The new layout
 * @return 1 if successful, 0 otherwise
 */
int vkz_transition_image_layout(VkImage image, VkFormat format,
  VkImageLayout old_layout,
  VkImageLayout new_layout);

/**
 * @brief Copy a buffer to an image
 * @param buffer The buffer
 * @param image  The image
 * @param width  The width of the image
 * @param height The height of the image
 * @return 1 if successful, 0 otherwise
 */
int vkz_copy_buffer_to_image(VkBuffer buffer, VkImage image,
  uint32_t width, uint32_t height);

/**
 * @brief Create a Vulkan image view
 * @param image_view   The image view to be created
 * @param image        The image for which the image view will be created
 * @param format       The image view format
 * @param aspect_flags The aspect flags (mask) e.g. VK_IMAGE_ASPECT_COLOR_BIT
 * @return 1 if successful, 0 otherwise
 */
int vkz_create_image_view(VkImageView* image_view, VkImage image,
  VkFormat format, VkImageAspectFlags aspect_flags);

/**
 * @brief Create a Vulkan sampler
 * @param sampler The sampler to be created
 * @return 1 if successful, 0 otherwise
 */
int vkz_create_sampler(VkSampler* sampler);

/**
 * @brief  Cleanup the memory, destroy any debug callbacks, as well as the
 *         logical device and the Vulkan instance.
 * @return 1 if successful, 0 otherwise
 */
int vkz_shutdown(void);

#endif //VKZOS_H
