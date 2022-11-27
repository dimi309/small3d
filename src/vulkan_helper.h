/**
 * @file vulkan_helper.h
 * @brief Vulkan helper functions.
 *
 * Created on: 2018/05/01
 * License: MIT
 * Copyright 2017 - 2022 Dimitri Kourkoulis
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef VULKAN_HELPER_H
#define VULKAN_HELPER_H

#ifdef __ANDROID__
#include <android_native_app_glue.h>
#include <android/asset_manager.h>

#define VK_USE_PLATFORM_ANDROID_KHR
#endif

#ifdef VULKAN_HELPER_IOS
#include <MoltenVK/mvk_vulkan.h>
#else
#include <vulkan/vulkan.h>
#endif

#ifdef SMALL3D_USING_XCODE
// Needed for compiling within Xcode (see extern "C" at the top of the file)
extern "C" {
#endif
  
/**
 * @brief Has a pipeline just been created or recreated?
 */

extern uint32_t vh_new_pipeline_state;

/**
 * @brief The Vulkan instance
 */
extern VkInstance vh_instance;

/**
 * @brief The surface on which graphics will be presented.
 */
extern VkSurfaceKHR vh_surface;

/**
 * @brief The physical device that is selected vh_init().
 */

extern VkPhysicalDevice vh_physical_device;

/**
 * @brief The logical device that is created by vh_init().
 */
extern VkDevice vh_logical_device;

/**
 * @brief The pipeline layouts, one for each pipeline created.
 */
extern VkPipelineLayout* vh_pipeline_layout;

/**
 * @brief Number of images in the swapchain, set by vh_create_swapchain().
 */
extern uint32_t vh_swapchain_image_count;

/**
 * @brief The colour used to clear the screen.
 */
extern VkClearColorValue vh_clear_colour;

/**
 * @brief The image used for shadow mapping.
 */
extern VkImage vh_shadow_image;

/**
 * @brief The image memory used for shadow mapping.
 */
extern VkDeviceMemory vh_shadow_image_memory;

/**
 * @brief The image view used for shadow mapping.
 */
extern VkImageView vh_shadow_image_view;

extern VkCommandBuffer cmd_buffer_copy_depth_to_shadow;

/**
 * @brief Copy the depth image to the shadow image.
 */
int vh_copy_depth_to_shadow_image();

/**
 * @brief Wait on a GPU-CPU fence
 * @param idx The GPU-CPU fence index
 */
void vh_wait_gpu_cpu_fence(uint32_t idx);

/**
 * @brief  Create a vulkan instance
 * @param  application_name The name of the application that will be using Vulkan
 * @param  enabled_extension_names Names of extensions used
 * @param  enabled_extension_count Number of extensions used
 * @return 1 if successful, 0 otherwise
 */
int vh_create_instance(const char* application_name,
                       const char** enabled_extension_names,
                       size_t enabled_extension_count);

/**
 * @brief  Clear the depth image.
 * @param  command_buffer The command buffer to record the clearing command to
 * @return 1 if successful, 0 otherwise
 */
int vh_clear_depth_image(VkCommandBuffer* command_buffer);

/**
 * @brief  Initialise. Internally this means create physical device, select queue
 *         families and create logical device.
 * @param  max_frames Maximum number of frames to be rendering to asynchronously
 * @return 1 if successful, 0 otherwise
 */
int vh_init(uint32_t max_frames);

/**
 * @brief Set width and height for all rendering calculations.
 * @param width The new width
 * @param height The new height
 * @return 1 if successful, 0 otherwise
 */
int vh_set_width_height(const uint32_t width, const uint32_t height);

/**
 * @brief Create the swapchain that will be used. This will also create
 * the associated image views.
 * @return 1 if successful, 0 otherwise
 */
int vh_create_swapchain();

/**
 * @brief Destroy the swapchain.
 * @return 1 if successful, 0 otherwise
 */
int vh_destroy_swapchain(void);

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
int vh_create_pipeline(const char* vertex_shader_path, const char* fragment_shader_path,
                       int (*set_input_state)(VkPipelineVertexInputStateCreateInfo*),
                       int (*set_pipeline_layout)(VkPipelineLayoutCreateInfo*), uint32_t* index);

/**
 * @brief Destroy a pipeline
 * @param index The index of the pipeline to destroy
 */
int vh_destroy_pipeline(uint32_t index);

/**
 * @brief Begin creating a drawing command buffer 
 *
 * @param command_buffer Pointer to the command buffer.
 * @return 1 if successful, 0 otherwise
 */
int vh_begin_draw_command_buffer(VkCommandBuffer* command_buffer);

/**
 * @brief Bind a pipeline to a command buffer
 *
 * @param pipeline_index The index of the pipeline
 * @param command_buffer Pointer to the command buffer.
 * @return 1 if successful, 0 otherwise
 */
int vh_bind_pipeline_to_command_buffer(uint32_t pipeline_index,
                                       const VkCommandBuffer* command_buffer);

/**
 * @brief Finish recording a command buffer.
 *
 * @param command_buffer Pointer to the command buffer.
 * @return 1 if successful, 0 otherwise
 */
int vh_end_draw_command_buffer(VkCommandBuffer* command_buffer);

/**
 * @brief Destroy a command buffer.
 *
 * @param command_buffer Pointer to the command buffer.
 * @return 1 if successful, 0 otherwise
 */
int vh_destroy_draw_command_buffer(VkCommandBuffer* command_buffer);

/**
 * @brief Create sync objects 
 * @return 1 if successful, 0 otherwise
 */
int vh_create_sync_objects(void);

/**
 * @brief Destroy sync objects
 * @return 1 if successful, 0 otherwise
 */
int vh_destroy_sync_objects(void);

/**
 * @brief Recreate the swapchain and the pipelines.
 * @return 1 if successful, 0 otherwise
 */
int vh_recreate_pipelines_and_swapchain(void);

/**
 * @brief Acquire next swapchain image
 *
 * @param pipeline_index The index of the pipeline
 * @param image_index    The index of the acquired swapchain image
 * @param frame_index_out [out] The frame index (used for async rendering - different from image_index)
 * @return 1 if successful, 0 otherwise
 */
int vh_acquire_next_image(uint32_t pipeline_index, uint32_t* image_index, uint32_t* frame_index_out);

/**
 * @brief Present next swapchain image (the one acquired by
 *        vh_acquire_next_image())
 * @return 1 if successful, 0 otherwise
 */
int vh_present_next_image(void);

/**
 * @brief  Send draw commands (will take effect on the current pipeline image
 *         acquired by vh_acquire_next_image())
 * @param  command_buffer Pointer to the command buffer containing the commands.
 * @param  only_shadows Only drawing shadows?
 * @return 1 if successful, 0 otherwise
 */
int vh_draw(VkCommandBuffer* command_buffer, int only_shadows);

/**
 * @brief  Create a buffer
 * @param  buffer                Variable in which to store the buffer
 * @param  buffer_usage_flags    Vulkan usage flags for the buffer
 * @param  buffer_size           The size of the buffer, in bytes
 * @param  memory                The GPU memory reserved for the buffer
 * @param  memory_property_flags Vulkan properties of the reserved memory
 * @return 1 if successful, 0 otherwise
 */
int vh_create_buffer(VkBuffer* buffer,
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
int vh_destroy_buffer(VkBuffer buffer, VkDeviceMemory memory);

/**
 * @brief Copy a buffer to another
 * @param source         The source buffer
 * @param destination    The destination buffer
 * @param size           The size of the buffer to be copied, in bytes
 * @return 1 if successful, 0 otherwise
 */
int vh_copy_buffer(VkBuffer source, VkBuffer destination, VkDeviceSize size);

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
int vh_create_image(VkImage* image,
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
int vh_destroy_image(VkImage image, VkDeviceMemory image_memory);

/**
 * @brief Transition the layout of a Vulkan image
 * @param image          The Vulkan image
 * @param format         The image format
 * @param old_layout     The old layout
 * @param new_layout     The new layout
 * @param only_depth     Only a depth image?
 * @return 1 if successful, 0 otherwise
 */
int vh_transition_image_layout(VkImage image, VkFormat format,
                               VkImageLayout old_layout,
                               VkImageLayout new_layout,
                               int only_depth);

/**
 * @brief Copy a buffer to an image
 * @param buffer The buffer
 * @param image  The image
 * @param width  The width of the image
 * @param height The height of the image
 * @return 1 if successful, 0 otherwise
 */
int vh_copy_buffer_to_image(VkBuffer buffer, VkImage image,
                            uint32_t width, uint32_t height);

/**
 * @brief Create a Vulkan image view
 * @param image_view   The image view to be created
 * @param image        The image for which the image view will be created
 * @param format       The image view format
 * @param aspect_flags The aspect flags (mask) e.g. VK_IMAGE_ASPECT_COLOR_BIT
 * @return 1 if successful, 0 otherwise
 */
int vh_create_image_view(VkImageView* image_view, VkImage image,
                         VkFormat format, VkImageAspectFlags aspect_flags);

/**
 * @brief Create a Vulkan sampler
 * @param sampler The sampler to be created
 * @return 1 if successful, 0 otherwise
 */
int vh_create_sampler(VkSampler* sampler, VkSamplerAddressMode addressMode);

/**
 * @brief Create command buffer that copies depth to
 *        shadow image.
 */
int vh_create_depth_to_shadow_copy_cmd();

/**
* @brief Destroy command buffer that copies depth to
*        shadow image.
*/
int vh_destroy_depth_to_shadow_copy_cmd();

/**
 * @brief  Cleanup the memory, destroy any debug callbacks, as well as the
 *         logical device and the Vulkan instance.
 * @return 1 if successful, 0 otherwise
 */
int vh_shutdown(void);

#ifdef SMALL3D_USING_XCODE  
// Needed for compiling within Xcode (see extern "C" at the top of the file)
}
#endif

#endif //VULKAN_HELPER_H
