#if (CPU_ && DEF_)
#define VK_NO_PROTOTYPES
#define VK_USE_PLATFORM_WAYLAND_KHR
#include "vulkan.h"

#define VK_CORE_FUNCTIONS \
  X(vkCreateInstance) \
  X(vkEnumerateInstanceLayerProperties) \
  X(vkEnumerateInstanceExtensionProperties) \
  X(vkGetInstanceProcAddr) \
  X(vkEnumeratePhysicalDevices) \
  X(vkGetPhysicalDeviceProperties) \
  X(vkGetPhysicalDeviceQueueFamilyProperties) \
  X(vkGetPhysicalDeviceSurfaceSupportKHR) \
  X(vkCreateDevice) \
  X(vkGetPhysicalDeviceSurfaceFormatsKHR) \
  X(vkGetPhysicalDeviceSurfaceCapabilitiesKHR) \
  X(vkGetPhysicalDeviceSurfacePresentModesKHR) \
  X(vkCreateSwapchainKHR) \
  X(vkCreateCommandPool) \
  X(vkAllocateCommandBuffers) \
  X(vkDestroySwapchainKHR) \
  X(vkDestroySurfaceKHR) \
  X(vkBeginCommandBuffer) \
  X(vkEndCommandBuffer) \
  X(vkQueueSubmit) \
  X(vkGetSwapchainImagesKHR) \
  X(vkCreateFence) \
  X(vkCmdPipelineBarrier2) \
  X(vkWaitForFences) \
  X(vkResetFences) \
  X(vkResetCommandPool) \
  X(vkCreateImageView) \
  X(vkGetPhysicalDeviceFeatures2) \
  X(vkGetDeviceQueue) \
  X(vkCreatePipelineLayout) \
  X(vkCreateGraphicsPipelines) \
  X(vkDestroyShaderModule) \
  X(vkCreateShaderModule) \
  X(vkCreateSemaphore) \
  X(vkAcquireNextImageKHR) \
  X(vkQueuePresentKHR) \
  X(vkCmdBeginRendering) \
  X(vkCmdSetViewport) \
  X(vkCmdBindPipeline) \
  X(vkCmdSetCullMode) \
  X(vkCmdSetFrontFace) \
  X(vkCmdSetPrimitiveTopology) \
  X(vkCmdEndRendering) \
  X(vkCmdSetScissor) \
  X(vkDestroyImageView) \
  X(vkDeviceWaitIdle) \
  X(vkQueueWaitIdle) \
  X(vkCreateBuffer) \
  X(vkGetBufferMemoryRequirements) \
  X(vkAllocateMemory) \
  X(vkMapMemory) \
  X(vkUnmapMemory) \
  X(vkGetPhysicalDeviceMemoryProperties) \
  X(vkBindBufferMemory) \
  X(vkCmdBindVertexBuffers) \
  X(vkCmdDraw) \
  X(vkCreateWaylandSurfaceKHR) \
  X(vkGetPhysicalDeviceWaylandPresentationSupportKHR) \
  X(vkCmdPushConstants) \
  X(vkCreateImage) \
  X(vkGetImageMemoryRequirements) \
  X(vkBindImageMemory) \
  X(vkCreateSampler) \
  X(vkCmdCopyBufferToImage) \
  X(vkCreateDescriptorSetLayout) \
  X(vkDestroyImage) \
  X(vkDestroySampler) \
  X(vkDestroyDescriptorSetLayout) \
  X(vkDestroyBuffer) \
  X(vkDestroyCommandPool) \
  X(vkFreeMemory) \
  X(vkDestroyFence) \
  X(vkGetDeviceProcAddr)

#define VK_EXTENSION_FUNCTIONS \
  X(vkCreateDebugReportCallbackEXT) \
  X(vkDestroyDebugReportCallbackEXT) \
  X(vkDebugReportMessageEXT)

#define X(name) Global PFN_##name name = 0;
VK_CORE_FUNCTIONS
VK_EXTENSION_FUNCTIONS
#undef X
#endif

#define MAX_RECTANGLE_COUNT 1024

#if (CPU_ && TYP_)

typedef struct GFX_Per_Frame GFX_Per_Frame;
struct GFX_Per_Frame {
  VkFence queue_submit_fence;
  VkCommandPool command_pool;
  VkCommandBuffer command_buffer;
  VkSemaphore swapchain_acquire_semaphore;
  VkSemaphore swapchain_release_semaphore;

  VkBuffer instance_buffer;
  VkDeviceMemory instance_buffer_memory;
  void *rect_instances;
  L1 rect_instances_count;
};

typedef struct GFX_Window GFX_Window;
struct GFX_Window {
  GFX_Window *next;

  VkSurfaceKHR surface;
  I1 surface_format_count;
  VkSurfaceFormatKHR *surface_formats;
  I1 present_mode_count;
  VkPresentModeKHR *present_modes;
  VkSwapchainKHR swapchain;
  VkExtent2D swapchain_extent;
  I1 image_count;
  VkImage *swapchain_images;
  VkImageView *swapchain_image_views;
  GFX_Per_Frame *per_frame;
};

typedef struct GFX_Rect_Instance GFX_Rect_Instance;
struct GFX_Rect_Instance {
  F4 dst_rect;
  F4 src_rect;
  F4 colors[4];
  F4 corner_radii;
  F4 border_color;
  F1 border_width;
  F1 softness;
  F1 omit_texture;
};

typedef struct GFX_Texture GFX_Texture;
struct GFX_Texture {
  GFX_Texture *next;

  VkImage image;
  VkImageView image_view;
  VkDeviceMemory memory;
  I1 width;
  I1 height;
};

typedef struct GFX_Batch GFX_Batch;
struct GFX_Batch {
  GFX_Batch *next;

  F4 clip_rect;
  GFX_Texture *texture;

  GFX_Rect_Instance *instances;
  L1 instance_count;
};

typedef struct GFX_BatchList GFX_BatchList;
struct GFX_BatchList {
  GFX_Batch *first;
  GFX_Batch *last;
};

typedef struct GFX_State GFX_State;
struct GFX_State {
  Arena *arena;

  VkInstance instance;
  VkDebugReportCallbackEXT vk_debug_callback;
  VkPhysicalDevice physical_device;
  VkPhysicalDeviceProperties physical_device_properties;
  VkDevice device;
  VkQueue queue;
  L1 present_queue_index;
  VkPipelineLayout pipeline_layout;
  VkPipeline pipeline;

  I1 image_idx;

  L1 recycle_semaphores_count;
  VkSemaphore recycle_semaphores[16];

  GFX_Window *first_free_window;
  GFX_Texture *first_free_texture;

  VkSampler texture_sampler;
  VkDescriptorSetLayout descriptor_set_layout;
  VkCommandPool upload_command_pool;
  GFX_Texture *white_texture;
};

#endif

#if (CPU_ && ROM_)

Global GFX_State *gfx_state = 0;
Global PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSetKHR = 0;

VKAPI_ATTR VkBool32 VKAPI_CALL debug_report_callback(VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT object_type, uint64_t object, size_t location,
    int32_t message_code, const char* layer_prefix, const char* message, void* user_data) {
  printf("%s - %s\n", layer_prefix, message);
  return VK_FALSE;
}

F4 oklch(F1 l, F1 c, F1 h, F1 alpha) {
  F1 h_rad = h * (PI / 180.0f);
  F4 result = { l, c, h_rad, alpha };
  return result;
}

Internal I1 gfx_find_memory_type(VkMemoryRequirements mem_reqs, VkMemoryPropertyFlags required_properties) {
  VkPhysicalDeviceMemoryProperties mem_properties;
  vkGetPhysicalDeviceMemoryProperties(gfx_state->physical_device, &mem_properties);

  for (I1 i = 0; i < mem_properties.memoryTypeCount; i++) {
    if ((mem_reqs.memoryTypeBits & (1 << i)) &&
        (mem_properties.memoryTypes[i].propertyFlags & required_properties) == required_properties) {
      return i;
    }
  }
  Assert(0); // Memory type not found
  return 0;
}

Internal void gfx_vk_transition_image_layout(
  VkCommandBuffer cmd,
  VkImage image,
  VkImageLayout old_layout,
  VkImageLayout new_layout,
  VkAccessFlags2 src_access_mask,
  VkAccessFlags2 dst_access_mask,
  VkPipelineStageFlags2 src_stage,
  VkPipelineStageFlags2 dst_stage) {
  VkImageMemoryBarrier2 image_barrier = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
    .srcStageMask = src_stage,
    .srcAccessMask = src_access_mask,
    .dstStageMask = dst_stage,
    .dstAccessMask = dst_access_mask,

    .oldLayout = old_layout,
    .newLayout = new_layout,

    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,

    .image = image,

    .subresourceRange = {
      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .baseMipLevel = 0,
      .levelCount = 1,
      .baseArrayLayer = 0,
      .layerCount = 1,
    },
  };

  VkDependencyInfo dependency_info = {
    .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
    .dependencyFlags = 0,
    .imageMemoryBarrierCount = 1,
    .pImageMemoryBarriers = &image_barrier,
  };

  vkCmdPipelineBarrier2(cmd, &dependency_info);
}

Internal GFX_Texture *gfx_tex2d_alloc(I1 width, I1 height, void *pixels) {
  Assert(width > 0 && height > 0);

  GFX_Texture *texture = gfx_state->first_free_texture;
  if (texture == 0) {
    texture = push_array(gfx_state->arena, GFX_Texture, 1);
  } else {
    SLLStackPop(gfx_state->first_free_texture);
    MemoryZeroStruct(texture);
  }

  VkResult result;
  L1 image_size = width * height * 4; // RGBA8

  ////////////////////////////////
  //~ kti: Create staging buffer

  VkBuffer staging_buffer;
  VkDeviceMemory staging_memory;

  VkBufferCreateInfo staging_ci = {
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .size = image_size,
    .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
  };
  result = vkCreateBuffer(gfx_state->device, &staging_ci, 0, &staging_buffer);
  Assert(result == VK_SUCCESS);

  VkMemoryRequirements staging_mem_reqs;
  vkGetBufferMemoryRequirements(gfx_state->device, staging_buffer, &staging_mem_reqs);

  VkMemoryAllocateInfo staging_alloc_info = {
    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .allocationSize = staging_mem_reqs.size,
    .memoryTypeIndex = gfx_find_memory_type(staging_mem_reqs,
                                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
  };
  result = vkAllocateMemory(gfx_state->device, &staging_alloc_info, 0, &staging_memory);
  Assert(result == VK_SUCCESS);

  result = vkBindBufferMemory(gfx_state->device, staging_buffer, staging_memory, 0);
  Assert(result == VK_SUCCESS);

  void *staging_ptr;
  result = vkMapMemory(gfx_state->device, staging_memory, 0, image_size, 0, &staging_ptr);
  Assert(result == VK_SUCCESS);
  memmove(staging_ptr, pixels, image_size);
  vkUnmapMemory(gfx_state->device, staging_memory);

  ////////////////////////////////
  //~ kti: Create texture image

  VkImageCreateInfo image_ci = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    .imageType = VK_IMAGE_TYPE_2D,
    .format = VK_FORMAT_R8G8B8A8_SRGB,
    .extent = {width, height, 1},
    .mipLevels = 1,
    .arrayLayers = 1,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .tiling = VK_IMAGE_TILING_OPTIMAL,
    .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  };
  result = vkCreateImage(gfx_state->device, &image_ci, 0, &texture->image);
  Assert(result == VK_SUCCESS);

  VkMemoryRequirements image_mem_reqs;
  vkGetImageMemoryRequirements(gfx_state->device, texture->image, &image_mem_reqs);

  VkMemoryAllocateInfo image_alloc_info = {
    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .allocationSize = image_mem_reqs.size,
    .memoryTypeIndex = gfx_find_memory_type(image_mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
  };
  result = vkAllocateMemory(gfx_state->device, &image_alloc_info, 0, &texture->memory);
  Assert(result == VK_SUCCESS);

  result = vkBindImageMemory(gfx_state->device, texture->image, texture->memory, 0);
  Assert(result == VK_SUCCESS);

  ////////////////////////////////
  //~ kti: Upload via command buffer

  VkCommandBuffer cmd;
  VkCommandBufferAllocateInfo cmd_alloc_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .commandPool = gfx_state->upload_command_pool,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = 1,
  };
  result = vkAllocateCommandBuffers(gfx_state->device, &cmd_alloc_info, &cmd);
  Assert(result == VK_SUCCESS);

  VkCommandBufferBeginInfo begin_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };
  result = vkBeginCommandBuffer(cmd, &begin_info);
  Assert(result == VK_SUCCESS);

  gfx_vk_transition_image_layout(cmd, texture->image,
    VK_IMAGE_LAYOUT_UNDEFINED,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    0, VK_ACCESS_2_TRANSFER_WRITE_BIT,
    VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
    VK_PIPELINE_STAGE_2_TRANSFER_BIT);

  VkBufferImageCopy region = {
    .bufferOffset = 0,
    .bufferRowLength = 0,
    .bufferImageHeight = 0,
    .imageSubresource = {
      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .mipLevel = 0,
      .baseArrayLayer = 0,
      .layerCount = 1,
    },
    .imageOffset = {0, 0, 0},
    .imageExtent = {width, height, 1},
  };
  vkCmdCopyBufferToImage(cmd, staging_buffer, texture->image,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

  gfx_vk_transition_image_layout(cmd, texture->image,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_ACCESS_2_SHADER_READ_BIT,
    VK_PIPELINE_STAGE_2_TRANSFER_BIT,
    VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);

  result = vkEndCommandBuffer(cmd);
  Assert(result == VK_SUCCESS);

  VkSubmitInfo submit = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .commandBufferCount = 1,
    .pCommandBuffers = &cmd,
  };
  result = vkQueueSubmit(gfx_state->queue, 1, &submit, VK_NULL_HANDLE);
  Assert(result == VK_SUCCESS);

  vkQueueWaitIdle(gfx_state->queue);

  ////////////////////////////////
  //~ kti: Cleanup staging resources

  vkDestroyBuffer(gfx_state->device, staging_buffer, 0);
  vkFreeMemory(gfx_state->device, staging_memory, 0);

  // Reset command pool to free command buffer
  vkResetCommandPool(gfx_state->device, gfx_state->upload_command_pool, 0);

  ////////////////////////////////
  //~ kti: Create image view

  VkImageViewCreateInfo view_ci = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    .image = texture->image,
    .viewType = VK_IMAGE_VIEW_TYPE_2D,
    .format = VK_FORMAT_R8G8B8A8_SRGB,
    .subresourceRange = {
      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .baseMipLevel = 0,
      .levelCount = 1,
      .baseArrayLayer = 0,
      .layerCount = 1,
    },
  };
  result = vkCreateImageView(gfx_state->device, &view_ci, 0, &texture->image_view);
  Assert(result == VK_SUCCESS);

  ////////////////////////////////
  //~ kti: Store dimensions

  texture->width = width;
  texture->height = height;

  return texture;
}

Internal void gfx_tex2d_free(GFX_Texture *tex) {
  vkDestroyImage(gfx_state->device, tex->image, 0);
  vkDestroyImageView(gfx_state->device, tex->image_view, 0);
  vkFreeMemory(gfx_state->device, tex->memory, 0);
  tex->width = 0;
  tex->height = 0;

  SLLStackPush(gfx_state->first_free_texture, tex);
}

Internal void gfx_init() {
  Arena *arena = arena_create(MiB(64));
  gfx_state = push_array(arena, GFX_State, 1);
  gfx_state->arena = arena;

  ////////////////////////////////
  //~ kti: Load Core Procedures

  void *lib = os_library_open(Str8_("libvulkan.so.1"));
  Assert(lib != 0);

#define X(name) *(void **)&name = os_library_load_proc(lib, Str8_(#name));
  VK_CORE_FUNCTIONS
#undef X

  ////////////////////////////////
  //~ kti: Create Instance

  VkApplicationInfo app_info = {
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pApplicationName = "m",
    .engineVersion = 1,
    .apiVersion = VK_API_VERSION_1_3,
  };

  I1 layer_count = 0;
  vkEnumerateInstanceLayerProperties(&layer_count, 0);
  Assert(layer_count > 0);

  VkLayerProperties *layers = push_array(arena, VkLayerProperties, layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, layers);

  L1 found_validation = 0;
  for EachIndex(i, layer_count) {
    if (cstr_compare(layers[i].layerName, "VK_LAYER_KHRONOS_validation")) {
      found_validation = 1;
    }
  }

  I1 extension_count = 0;
  vkEnumerateInstanceExtensionProperties(0, &extension_count, 0);
  VkExtensionProperties *extensions = push_array(arena, VkExtensionProperties, extension_count);
  vkEnumerateInstanceExtensionProperties(0, &extension_count, extensions);

  const char *required_extensions[] = {"VK_KHR_surface", "VK_KHR_wayland_surface", "VK_EXT_debug_report"};
  I1 found_extension_count = 0;
  for EachIndex(i, extension_count) {
    for EachIndex(j, ArrayCount(required_extensions)) {
      if (cstr_compare(required_extensions[j], extensions[i].extensionName)) {
        found_extension_count += 1;
      }
    }
  }

  Assert(found_extension_count == ArrayCount(required_extensions));

  VkInstanceCreateInfo inst_info = {
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pApplicationInfo = &app_info,
    .enabledExtensionCount = ArrayCount(required_extensions),
    .ppEnabledExtensionNames = required_extensions,
  };

  const char *layer_names[] = { "VK_LAYER_KHRONOS_validation" };
  if (found_validation) {
    inst_info.enabledLayerCount = ArrayCount(layer_names);
    inst_info.ppEnabledLayerNames = layer_names;
  }

  VkResult result = vkCreateInstance(&inst_info, 0, &gfx_state->instance);
  Assert(result == VK_SUCCESS);

  ////////////////////////////////
  //~ kti: Load Extension Procedures

#define X(name) *(void **)&name = vkGetInstanceProcAddr(gfx_state->instance, #name);
  VK_EXTENSION_FUNCTIONS
#undef X

  ////////////////////////////////
  //~ kti: Register Debug Report Callback

  VkDebugReportCallbackCreateInfoEXT callback_ci = {
    .sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT,
    .flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
    .pfnCallback = &debug_report_callback,
    .pUserData = 0,
  };

  result = vkCreateDebugReportCallbackEXT(gfx_state->instance, &callback_ci, 0, &gfx_state->vk_debug_callback);
  Assert(result == VK_SUCCESS);

  ////////////////////////////////
  //~ kti: Physical Device

  I1 device_count = 0;
  vkEnumeratePhysicalDevices(gfx_state->instance, &device_count, 0);
  Assert(device_count > 0);

  VkPhysicalDevice *physical_devices = push_array(arena, VkPhysicalDevice, device_count);
  vkEnumeratePhysicalDevices(gfx_state->instance, &device_count, physical_devices);

  for EachIndex(i, device_count) {
    VkPhysicalDeviceProperties props = {0};
    vkGetPhysicalDeviceProperties(physical_devices[i], &props);

    I1 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_devices[i], &queue_family_count, 0);
    VkQueueFamilyProperties *queue_family_properties = push_array(arena, VkQueueFamilyProperties, queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_devices[i], &queue_family_count, queue_family_properties);

    for EachIndex(j, queue_family_count) {
      // VkBool32 supports_present = 0;
      // vkGetPhysicalDeviceSurfaceSupportKHR(physical_devices[i], j, )
      if (queue_family_properties[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        gfx_state->physical_device = physical_devices[i];
        gfx_state->physical_device_properties = props;
        gfx_state->present_queue_index = j;
        break;
      }
    }

    if (gfx_state->physical_device) {
      break;
    }
  }

  Assert(gfx_state->physical_device);

  VkPhysicalDeviceFeatures2 query_device_features2 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
  VkPhysicalDeviceVulkan13Features query_vulkan13_features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
  VkPhysicalDeviceExtendedDynamicStateFeaturesEXT query_extended_dynamic_state_features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT};
  query_device_features2.pNext  = &query_vulkan13_features;
  query_vulkan13_features.pNext = &query_extended_dynamic_state_features;

  vkGetPhysicalDeviceFeatures2(gfx_state->physical_device, &query_device_features2);

  //- kti: Check if physical device supports Vulkan 1.3 features
  Assert(query_vulkan13_features.dynamicRendering);
  Assert(query_vulkan13_features.synchronization2);
  Assert(query_extended_dynamic_state_features.extendedDynamicState);

  ////////////////////////////////
  //~ kti: Logical device

  VkPhysicalDeviceExtendedDynamicStateFeaturesEXT enable_extended_dynamic_state_features = {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
    .extendedDynamicState = VK_TRUE
  };

  VkPhysicalDeviceVulkan13Features enable_vulkan13_features = {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
    .pNext = &enable_extended_dynamic_state_features,
    .synchronization2 = VK_TRUE,
    .dynamicRendering = VK_TRUE,
  };

  VkPhysicalDeviceFeatures2 enable_device_features2 = {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
    .pNext = &enable_vulkan13_features,
    .features = {
      .shaderClipDistance = VK_TRUE, // TODO: Do we even need this?
    },
  };

  float queue_priorities[] = { 1.0f };
  VkDeviceQueueCreateInfo queue_ci = {
    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
    .queueFamilyIndex = gfx_state->present_queue_index,
    .queueCount = 1,
    .pQueuePriorities = queue_priorities,
  };

  const char *device_extensions[] = { "VK_KHR_swapchain", "VK_KHR_push_descriptor" };
  VkDeviceCreateInfo device_info = {
    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .pNext = &enable_device_features2,
    .queueCreateInfoCount = 1,
    .pQueueCreateInfos = &queue_ci,

    .enabledLayerCount = ArrayCount(layer_names),
    .ppEnabledLayerNames = layer_names,

    .enabledExtensionCount = ArrayCount(device_extensions),
    .ppEnabledExtensionNames = device_extensions,
  };

  result = vkCreateDevice(gfx_state->physical_device, &device_info, 0, &gfx_state->device);
  Assert(result == VK_SUCCESS);

  vkGetDeviceQueue(gfx_state->device, gfx_state->present_queue_index, 0, &gfx_state->queue);

  ////////////////////////////////
  //~ kti: Load Push Descriptor Function

  vkCmdPushDescriptorSetKHR =
    (PFN_vkCmdPushDescriptorSetKHR)vkGetDeviceProcAddr(gfx_state->device, "vkCmdPushDescriptorSetKHR");
  Assert(vkCmdPushDescriptorSetKHR != 0);

  ////////////////////////////////
  //~ kti: Pipeline

  VkPushConstantRange push_constants_range = {
    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
    .offset = 0,
    .size = sizeof(F1) * 4,
  };

  VkDescriptorSetLayoutBinding bindings[] = {
    {
      .binding = 0,
      .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .descriptorCount = 1,
      .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
    }
  };

  VkDescriptorSetLayoutCreateInfo descriptor_layout_ci = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
    .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR,
    .bindingCount = 1,
    .pBindings = bindings,
  };
  result = vkCreateDescriptorSetLayout(gfx_state->device, &descriptor_layout_ci, 0, &gfx_state->descriptor_set_layout);
  Assert(result == VK_SUCCESS);

  VkPipelineLayoutCreateInfo layout_ci = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .setLayoutCount = 1,
    .pSetLayouts = &gfx_state->descriptor_set_layout,
    .pushConstantRangeCount = 1,
    .pPushConstantRanges = &push_constants_range,
  };
  result = vkCreatePipelineLayout(gfx_state->device, &layout_ci, 0, &gfx_state->pipeline_layout);
  Assert(result == VK_SUCCESS);

  VkVertexInputBindingDescription binding_description = {
    .binding = 0,
    .stride = sizeof(GFX_Rect_Instance),
    .inputRate = VK_VERTEX_INPUT_RATE_INSTANCE,
  };

  VkVertexInputAttributeDescription attribute_descriptions[] = {
    {.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(GFX_Rect_Instance, dst_rect)},
    {.location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(GFX_Rect_Instance, src_rect)},
    {.location = 2, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(GFX_Rect_Instance, colors[0])},
    {.location = 3, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(GFX_Rect_Instance, colors[1])},
    {.location = 4, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(GFX_Rect_Instance, colors[2])},
    {.location = 5, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(GFX_Rect_Instance, colors[3])},
    {.location = 6, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(GFX_Rect_Instance, corner_radii)},
    {.location = 7, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(GFX_Rect_Instance, border_color)},
    {.location = 8, .binding = 0, .format = VK_FORMAT_R32_SFLOAT,          .offset = offsetof(GFX_Rect_Instance, border_width)},
    {.location = 9, .binding = 0, .format = VK_FORMAT_R32_SFLOAT,          .offset = offsetof(GFX_Rect_Instance, softness)},
    {.location = 10, .binding = 0, .format = VK_FORMAT_R32_SFLOAT,          .offset = offsetof(GFX_Rect_Instance, omit_texture)},
  };

  VkPipelineVertexInputStateCreateInfo vertex_input = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .vertexBindingDescriptionCount = 1,
    .pVertexBindingDescriptions = &binding_description,
    .vertexAttributeDescriptionCount = ArrayCount(attribute_descriptions),
    .pVertexAttributeDescriptions = attribute_descriptions,
  };

  VkPipelineInputAssemblyStateCreateInfo input_assembly = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    .primitiveRestartEnable = VK_FALSE,
  };

  VkPipelineRasterizationStateCreateInfo raster = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .depthClampEnable = VK_FALSE,
    .rasterizerDiscardEnable = VK_FALSE,
    .polygonMode = VK_POLYGON_MODE_FILL,
    .depthBiasEnable = VK_FALSE,
    .lineWidth = 1.0f,
  };

  VkDynamicState dynamic_states[] = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR,
    VK_DYNAMIC_STATE_CULL_MODE,
    VK_DYNAMIC_STATE_FRONT_FACE,
    VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY,
  };

  VkPipelineColorBlendAttachmentState blend_attachment = {
    .blendEnable = VK_TRUE,
    .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
    .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    .colorBlendOp = VK_BLEND_OP_ADD,
    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
    .alphaBlendOp = VK_BLEND_OP_ADD,
    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                      VK_COLOR_COMPONENT_G_BIT |
                      VK_COLOR_COMPONENT_B_BIT |
                      VK_COLOR_COMPONENT_A_BIT,
  };

  VkPipelineColorBlendStateCreateInfo blend = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .attachmentCount = 1,
    .pAttachments = &blend_attachment,
  };

  VkPipelineViewportStateCreateInfo viewport = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .viewportCount = 1,
    .scissorCount = 1,
  };

  VkPipelineDepthStencilStateCreateInfo depth_stencil = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
    .depthCompareOp = VK_COMPARE_OP_ALWAYS,
  };

  VkPipelineMultisampleStateCreateInfo multisample = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
  };

  VkPipelineDynamicStateCreateInfo dynamic_state_ci = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
    .dynamicStateCount = ArrayCount(dynamic_states),
    .pDynamicStates = dynamic_states,
  };

  String8 vert_shader_code = os_read_entire_file(arena, Str8_("./shaders/shader.vert.spv"));
  String8 frag_shader_code = os_read_entire_file(arena, Str8_("./shaders/shader.frag.spv"));

  Assert(vert_shader_code.len != 0);
  Assert(frag_shader_code.len != 0);

  VkShaderModuleCreateInfo vert_shader_ci = {
    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .codeSize = vert_shader_code.len,
    .pCode = (const uint32_t *)vert_shader_code.str,
  };
  VkShaderModuleCreateInfo frag_shader_ci = {
    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .codeSize = frag_shader_code.len,
    .pCode = (const uint32_t *)frag_shader_code.str,
  };

  VkShaderModule vert_shader, frag_shader;

  result = vkCreateShaderModule(gfx_state->device, &vert_shader_ci, 0, &vert_shader);
  Assert(result == VK_SUCCESS);
  result = vkCreateShaderModule(gfx_state->device, &frag_shader_ci, 0, &frag_shader);
  Assert(result == VK_SUCCESS);

  VkPipelineShaderStageCreateInfo shader_stages[] = {
    {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_VERTEX_BIT,
      .module = vert_shader,
      .pName = "main",
    },
    {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
      .module = frag_shader,
      .pName = "main",
    },
  };

  VkFormat color_format = VK_FORMAT_B8G8R8A8_SRGB;
  VkPipelineRenderingCreateInfo pipeline_rendering = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
    .colorAttachmentCount = 1,
    .pColorAttachmentFormats = &color_format,
  };

  VkGraphicsPipelineCreateInfo pipeline_ci = {
    .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .pNext               = &pipeline_rendering,
    .stageCount          = ArrayCount(shader_stages),
    .pStages             = shader_stages,
    .pVertexInputState   = &vertex_input,
    .pInputAssemblyState = &input_assembly,
    .pViewportState      = &viewport,
    .pRasterizationState = &raster,
    .pMultisampleState   = &multisample,
    .pDepthStencilState  = &depth_stencil,
    .pColorBlendState    = &blend,
    .pDynamicState       = &dynamic_state_ci,
    .layout              = gfx_state->pipeline_layout,
    .renderPass          = VK_NULL_HANDLE,
    .subpass             = 0,
  };

  result = vkCreateGraphicsPipelines(gfx_state->device, VK_NULL_HANDLE, 1, &pipeline_ci, 0, &gfx_state->pipeline);

  vkDestroyShaderModule(gfx_state->device, vert_shader, 0);
  vkDestroyShaderModule(gfx_state->device, frag_shader, 0);

  ////////////////////////////////
  //~ kti: Sampler

  VkSamplerCreateInfo sampler_ci = {
    .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
    .magFilter = VK_FILTER_LINEAR,
    .minFilter = VK_FILTER_LINEAR,
    .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
    .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
    .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
    .anisotropyEnable = VK_FALSE,
    .maxAnisotropy = 1.0f,
    .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
    .unnormalizedCoordinates = VK_FALSE,
    .compareEnable = VK_FALSE,
    .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
  };
  result = vkCreateSampler(gfx_state->device, &sampler_ci, 0, &gfx_state->texture_sampler);
  Assert(result == VK_SUCCESS);

  ////////////////////////////////
  //~ kti: Upload Command Pool

  VkCommandPoolCreateInfo upload_pool_ci = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
    .queueFamilyIndex = gfx_state->present_queue_index,
  };
  result = vkCreateCommandPool(gfx_state->device, &upload_pool_ci, 0, &gfx_state->upload_command_pool);
  Assert(result == VK_SUCCESS);

  ////////////////////////////////
  //~ kti: White Texture

  I1 white = 0xFFFFFFFF;
  gfx_state->white_texture = gfx_tex2d_alloc(1, 1, &white);
}

Internal void gfx_vk_recreate_swapchain(OS_Window *os_window, GFX_Window *vkw) {
  vkDeviceWaitIdle(gfx_state->device);

  //- kti: Destroy old resources (if they exist).
  if (vkw->swapchain != VK_NULL_HANDLE) {
    vkDestroySwapchainKHR(gfx_state->device, vkw->swapchain, 0);
    vkw->swapchain = VK_NULL_HANDLE;
  }
  for EachIndex(i, vkw->image_count) {
    vkDestroyImageView(gfx_state->device, vkw->swapchain_image_views[i], 0);
  }

  //- kti: Our pipeline uses BGRA_SRGB so we force it. Not optimal.
  VkFormat color_format = VK_FORMAT_UNDEFINED;

  if (vkw->surface_format_count == 1 || vkw->surface_formats[0].format == VK_FORMAT_UNDEFINED) {
    color_format = VK_FORMAT_B8G8R8A8_SRGB;
  } else {
    for EachIndex(i, vkw->surface_format_count) {
      if (vkw->surface_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB) {
        color_format = VK_FORMAT_B8G8R8A8_SRGB;
        break;
      }
    }
  }
  Assert(color_format == VK_FORMAT_B8G8R8A8_SRGB);

  VkColorSpaceKHR color_space = vkw->surface_formats[0].colorSpace;

  VkSurfaceCapabilitiesKHR surface_capabilities = {0};
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gfx_state->physical_device, vkw->surface, &surface_capabilities);

  I1 desired_image_count = 2;
  if (desired_image_count < surface_capabilities.minImageCount) {
    desired_image_count = surface_capabilities.minImageCount;
  } else if (surface_capabilities.maxImageCount != 0 && desired_image_count > surface_capabilities.maxImageCount) {
    desired_image_count = surface_capabilities.maxImageCount;
  }

  VkExtent2D surface_resolution = surface_capabilities.currentExtent;
  if (surface_resolution.width == I1_MAX) {
    surface_resolution.width = os_window->width;
    surface_resolution.height = os_window->height;
  }

  VkSurfaceTransformFlagBitsKHR pre_transform = surface_capabilities.currentTransform;
  if(surface_capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
    pre_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  }

  //- kti: FIFO is always supported.
  VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
  for EachIndex(i, vkw->present_mode_count) {
    if (vkw->present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
      present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
      break;
    }
  }

  VkSwapchainCreateInfoKHR swpachain_ci = {
    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .surface = vkw->surface,
    .minImageCount = desired_image_count,
    .imageFormat = color_format,
    .imageColorSpace = color_space,
    .imageExtent = surface_resolution,
    .imageArrayLayers = 1,
    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .preTransform = pre_transform,
    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    .presentMode = present_mode,
    .clipped = VK_TRUE,
  };

  VkResult result = vkCreateSwapchainKHR(gfx_state->device, &swpachain_ci, 0, &vkw->swapchain);
  Assert(result == VK_SUCCESS);

  vkw->swapchain_extent = surface_resolution;

  ////////////////////////////////
  //~ kti: Image Views

  I1 prev_image_count = vkw->image_count;
  vkGetSwapchainImagesKHR(gfx_state->device, vkw->swapchain, &vkw->image_count, 0);
  if (prev_image_count == 0) {
    vkw->swapchain_images = push_array(gfx_state->arena, VkImage, vkw->image_count);
    vkw->swapchain_image_views = push_array(gfx_state->arena, VkImageView, vkw->image_count);
  } else if (prev_image_count != vkw->image_count) {
    Assert(0);
  }
  vkGetSwapchainImagesKHR(gfx_state->device, vkw->swapchain, &vkw->image_count, vkw->swapchain_images);

  for EachIndex(i, vkw->image_count) {
    VkImageViewCreateInfo image_view_ci = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = vkw->swapchain_images[i],
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = color_format,
      .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .subresourceRange.baseMipLevel = 0,
      .subresourceRange.levelCount = 1,
      .subresourceRange.baseArrayLayer = 0,
      .subresourceRange.layerCount = 1,
    };
    vkCreateImageView(gfx_state->device, &image_view_ci, 0, &vkw->swapchain_image_views[i]);
  }
}

Internal GFX_Window *gfx_window_equip(OS_Window *window) {
  GFX_Window *vkw = gfx_state->first_free_window;
  if (vkw == 0) {
    vkw = push_array(gfx_state->arena, GFX_Window, 1);
  } else {
    SLLStackPop(gfx_state->first_free_window);
    MemoryZeroStruct(vkw);
  }

  ////////////////////////////////
  //~ kti: Surface

  VkWaylandSurfaceCreateInfoKHR surface_ci = {
    .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
    .display = ramM.display,
    .surface = window->surface,
  };
  VkResult result = vkCreateWaylandSurfaceKHR(gfx_state->instance, &surface_ci, 0, &vkw->surface);
  Assert(result == VK_SUCCESS);

  VkBool32 supports = vkGetPhysicalDeviceWaylandPresentationSupportKHR(gfx_state->physical_device, gfx_state->present_queue_index, ramM.display);
  Assert(supports);

  vkGetPhysicalDeviceSurfaceFormatsKHR(gfx_state->physical_device, vkw->surface, &vkw->surface_format_count, 0);
  vkw->surface_formats = push_array(gfx_state->arena, VkSurfaceFormatKHR, vkw->surface_format_count);
  vkGetPhysicalDeviceSurfaceFormatsKHR(gfx_state->physical_device, vkw->surface, &vkw->surface_format_count, vkw->surface_formats);

  vkGetPhysicalDeviceSurfacePresentModesKHR(gfx_state->physical_device, vkw->surface, &vkw->present_mode_count, 0);
  vkw->present_modes = push_array(gfx_state->arena, VkPresentModeKHR, vkw->present_mode_count);
  vkGetPhysicalDeviceSurfacePresentModesKHR(gfx_state->physical_device, vkw->surface, &vkw->present_mode_count, vkw->present_modes);

  ////////////////////////////////
  //~ kti: Swapchain

  gfx_vk_recreate_swapchain(window, vkw);

  ////////////////////////////////
  //~ kti: Per Frame Data

  vkw->per_frame = push_array(gfx_state->arena, GFX_Per_Frame, vkw->image_count);

  for EachIndex(i, vkw->image_count) {
    GFX_Per_Frame *frame = &vkw->per_frame[i];

    //- kti: Fence

    VkFenceCreateInfo fence_ci = {
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };
    result = vkCreateFence(gfx_state->device, &fence_ci, 0, &frame->queue_submit_fence);
    Assert(result == VK_SUCCESS);

    //- kti: Command Buffer

    VkCommandPoolCreateInfo command_pool_ci = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
      .queueFamilyIndex = gfx_state->present_queue_index,
    };

    result = vkCreateCommandPool(gfx_state->device, &command_pool_ci, 0, &frame->command_pool);
    Assert(result == VK_SUCCESS);

    VkCommandBufferAllocateInfo command_buffer_alloc_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = frame->command_pool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1,
    };

    result = vkAllocateCommandBuffers(gfx_state->device, &command_buffer_alloc_info, &frame->command_buffer);
    Assert(result == VK_SUCCESS);

    //- kti: Instance Buffer

    VkBufferCreateInfo instance_buffer_ci = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = sizeof(GFX_Rect_Instance) * MAX_RECTANGLE_COUNT,
      .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    result = vkCreateBuffer(gfx_state->device, &instance_buffer_ci, 0, &frame->instance_buffer);
    Assert(result == VK_SUCCESS);

    VkMemoryRequirements memory_requirements = {0};
    vkGetBufferMemoryRequirements(gfx_state->device, frame->instance_buffer, &memory_requirements);

    VkMemoryAllocateInfo alloc_info = {
      .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize  = memory_requirements.size,
      .memoryTypeIndex = gfx_find_memory_type(memory_requirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
    };

    result = vkAllocateMemory(gfx_state->device, &alloc_info, 0, &frame->instance_buffer_memory);
    Assert(result == VK_SUCCESS);

    result = vkBindBufferMemory(gfx_state->device, frame->instance_buffer, frame->instance_buffer_memory, 0);
    Assert(result == VK_SUCCESS);

    result = vkMapMemory(gfx_state->device, frame->instance_buffer_memory, 0, instance_buffer_ci.size, 0, &frame->rect_instances);
    Assert(result == VK_SUCCESS);
  }

  return vkw;
}

Internal void gfx_window_unequip(GFX_Window *vkw) {
  ////////////////////////////////
  //~ kti: Destroy Per frame Resources

  //- kti: Wait for all gpu work to finich for this window.
  Temp_Arena scratch = temp_arena_begin(gfx_state->arena);
  VkFence *fences = push_array(scratch.arena, VkFence, vkw->image_count);
  for EachIndex(i, vkw->image_count) {
    fences[i] = vkw->per_frame[i].queue_submit_fence;
  }
  vkWaitForFences(gfx_state->device, vkw->image_count, fences, VK_TRUE, L1_MAX);
  temp_arena_end(scratch);

  for EachIndex(i, vkw->image_count) {
    GFX_Per_Frame *frame = &vkw->per_frame[i];

    if (frame->queue_submit_fence != VK_NULL_HANDLE) {
      vkDestroyFence(gfx_state->device, frame->queue_submit_fence, 0);
    }

    if (frame->command_pool != VK_NULL_HANDLE) {
      vkDestroyCommandPool(gfx_state->device, frame->command_pool, 0);
    }

    vkDestroyImageView(gfx_state->device, vkw->swapchain_image_views[i], 0);

    //- kti: Free instance buffers.
    if (frame->instance_buffer_memory != VK_NULL_HANDLE) {
      vkUnmapMemory(gfx_state->device, frame->instance_buffer_memory);
    }
    if (frame->instance_buffer != VK_NULL_HANDLE) {
      vkDestroyBuffer(gfx_state->device, frame->instance_buffer, 0);
    }
    if (frame->instance_buffer_memory != VK_NULL_HANDLE) {
      vkFreeMemory(gfx_state->device, frame->instance_buffer_memory, 0);
    }

    //- kti: Recycle semaphores instead of destroying.
    if (frame->swapchain_acquire_semaphore != VK_NULL_HANDLE) {
      gfx_state->recycle_semaphores[gfx_state->recycle_semaphores_count] = frame->swapchain_acquire_semaphore;
      gfx_state->recycle_semaphores_count += 1;
    }
    if (frame->swapchain_release_semaphore != VK_NULL_HANDLE) {
      gfx_state->recycle_semaphores[gfx_state->recycle_semaphores_count] = frame->swapchain_release_semaphore;
      gfx_state->recycle_semaphores_count += 1;
    }
  }

  ////////////////////////////////
  //~ kti: Destroy Per Window Resources

  vkDestroySwapchainKHR(gfx_state->device, vkw->swapchain, 0);
  vkDestroySurfaceKHR(gfx_state->instance, vkw->surface, 0);

  SLLStackPush(gfx_state->first_free_window, vkw);
}

Internal void gfx_window_begin_frame(OS_Window *os_window, GFX_Window *vkw) {
  ////////////////////////////////
  //~ kti: Acquire image

  //- kti: Check for window resize.
  if (os_window->width != vkw->swapchain_extent.width || os_window->height != vkw->swapchain_extent.height) {
    gfx_vk_recreate_swapchain(os_window, vkw);
  }

  //- kti: Grab a sempahore.
  VkSemaphore acquire_semaphore;
  if (gfx_state->recycle_semaphores_count == 0) {
    VkSemaphoreCreateInfo semaphore_ci = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    VkResult result = vkCreateSemaphore(gfx_state->device, &semaphore_ci, 0, &acquire_semaphore);
    Assert(result == VK_SUCCESS);
  } else {
    acquire_semaphore = gfx_state->recycle_semaphores[gfx_state->recycle_semaphores_count - 1];
    gfx_state->recycle_semaphores_count -= 1;
  }

  //- kti: Grab the next image.
  I1 image_idx = 0;
  VkResult img_acquire_result = VK_RESULT_MAX_ENUM;
  for EachIndex(try, 2) {
    img_acquire_result = vkAcquireNextImageKHR(gfx_state->device, vkw->swapchain, L1_MAX, acquire_semaphore, VK_NULL_HANDLE, &image_idx);
    if (img_acquire_result == VK_SUBOPTIMAL_KHR || img_acquire_result == VK_ERROR_OUT_OF_DATE_KHR) {
      gfx_vk_recreate_swapchain(os_window, vkw);
    } else {
      break;
    }
  }

  if (img_acquire_result != VK_SUCCESS) {
    // - kti: Recycle the semaphore we never used.
    Assert(gfx_state->recycle_semaphores_count < ArrayCount(gfx_state->recycle_semaphores));
    gfx_state->recycle_semaphores[gfx_state->recycle_semaphores_count] = acquire_semaphore;
    gfx_state->recycle_semaphores_count += 1;

    vkQueueWaitIdle(gfx_state->queue);
    return;
  }

  //- kti: Wait for previous work submitted to this image is complete.
  vkWaitForFences(gfx_state->device, 1, &vkw->per_frame[image_idx].queue_submit_fence, VK_TRUE, L1_MAX);
  vkResetFences(gfx_state->device, 1, &vkw->per_frame[image_idx].queue_submit_fence);

  vkResetCommandPool(gfx_state->device, vkw->per_frame[image_idx].command_pool, 0);

  //- kti: Recycle old semaphore
  VkSemaphore old_semaphore = vkw->per_frame[image_idx].swapchain_acquire_semaphore;
  if (old_semaphore != VK_NULL_HANDLE) {
    gfx_state->recycle_semaphores[gfx_state->recycle_semaphores_count] = old_semaphore;
    gfx_state->recycle_semaphores_count += 1;
  }

  vkw->per_frame[image_idx].swapchain_acquire_semaphore = acquire_semaphore;

  gfx_state->image_idx = image_idx;

  ////////////////////////////////
  //~ kti: Begin Rendering

  VkCommandBuffer cmd = vkw->per_frame[image_idx].command_buffer;
  VkCommandBufferBeginInfo begin_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };

  VkResult result = vkBeginCommandBuffer(cmd, &begin_info);
  Assert(result == VK_SUCCESS);

  gfx_vk_transition_image_layout(
    cmd,
    vkw->swapchain_images[image_idx],
    VK_IMAGE_LAYOUT_UNDEFINED,
    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    0,
    VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
    VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
    VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);

  VkClearValue clear_value = {.color= {{0.0f, 0.0f, 0.0f, 1.0f}}};

  VkRenderingAttachmentInfo color_attachment = {
    .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
    .imageView   = vkw->swapchain_image_views[image_idx],
    .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    .loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
    .clearValue  = clear_value,
  };

  VkRenderingInfo rendering_info = {
    .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
    .renderArea = {
      .offset = {0, 0},
      .extent = {
        .width = vkw->swapchain_extent.width,
        .height = vkw->swapchain_extent.height,
      }
    },
    .layerCount = 1,
    .colorAttachmentCount = 1,
    .pColorAttachments = &color_attachment
  };

  vkCmdBeginRendering(cmd, &rendering_info);

  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, gfx_state->pipeline);

  VkViewport viewport = {
    .width = vkw->swapchain_extent.width,
    .height = vkw->swapchain_extent.height,
    .minDepth = 0.0f,
    .maxDepth = 1.0f,
  };

  vkCmdSetViewport(cmd, 0, 1, &viewport);

  vkCmdSetCullMode(cmd, VK_CULL_MODE_NONE);
  vkCmdSetFrontFace(cmd, VK_FRONT_FACE_CLOCKWISE);
  vkCmdSetPrimitiveTopology(cmd, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

  vkw->per_frame[image_idx].rect_instances_count = 0;
}

Internal void gfx_window_submit(OS_Window *os_window, GFX_Window *vkw, GFX_BatchList batches) {
  I1 image_idx = gfx_state->image_idx;
  VkCommandBuffer cmd = vkw->per_frame[image_idx].command_buffer;
  VkBuffer instance_buffer = vkw->per_frame[image_idx].instance_buffer;
  GFX_Rect_Instance *rect_instances = (GFX_Rect_Instance *)vkw->per_frame[image_idx].rect_instances;
  L1 rect_instances_count = vkw->per_frame[image_idx].rect_instances_count;

  VkDeviceSize offset = {0};
  vkCmdBindVertexBuffers(cmd, 0, 1, &instance_buffer, &offset);

  for (GFX_Batch *batch = batches.first; batch != 0; batch = batch->next) {
    if (rect_instances_count + batch->instance_count > MAX_RECTANGLE_COUNT) {
      printf("Max number of rectangle instances reached for frame. Skipping batch.\n");
      continue;
    }

    GFX_Texture *texture = batch->texture;
    if (texture == 0) {
      texture = gfx_state->white_texture;
    }

    VkRect2D scissor = {
      .offset = {
        .x = 0,
        .y = 0,
      },
      .extent = {
        .width  = vkw->swapchain_extent.width,
        .height = vkw->swapchain_extent.height,
      },
    };
    if (batch->clip_rect.x != 0.0f || batch->clip_rect.y != 0.0f ||
        batch->clip_rect.z != 0.0f || batch->clip_rect.w != 0.0f) {
      scissor.offset.x = batch->clip_rect.x;
      scissor.offset.y = batch->clip_rect.y;
      scissor.extent.width = batch->clip_rect.z;
      scissor.extent.height = batch->clip_rect.w;
    }
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    VkDescriptorImageInfo image_info = {
      .sampler = gfx_state->texture_sampler,
      .imageView = texture->image_view,
      .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };

    VkWriteDescriptorSet write = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstBinding = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .pImageInfo = &image_info,
    };

    vkCmdPushDescriptorSetKHR(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, gfx_state->pipeline_layout, 0, 1, &write);

    F1 push_data[4] = {
      vkw->swapchain_extent.width, vkw->swapchain_extent.height,
      (F1)texture->width, (F1)texture->height
    };
    vkCmdPushConstants(cmd, gfx_state->pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(push_data), push_data);

    memmove(rect_instances + rect_instances_count, batch->instances, batch->instance_count*sizeof(GFX_Rect_Instance));

    vkCmdDraw(cmd, 6, batch->instance_count, 0, rect_instances_count);

    rect_instances_count += batch->instance_count;
  }

  vkw->per_frame[image_idx].rect_instances_count = rect_instances_count;

}

Internal void gfx_window_end_frame(OS_Window *os_window, GFX_Window *vkw) {
  I1 image_idx = gfx_state->image_idx;
  VkCommandBuffer cmd = vkw->per_frame[image_idx].command_buffer;

  ////////////////////////////////
  //~ kti: End Rendering

  vkCmdEndRendering(cmd);

  gfx_vk_transition_image_layout(
    cmd,
    vkw->swapchain_images[image_idx],
    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
    0,
    VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
    VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT);

  vkEndCommandBuffer(cmd);

  if (vkw->per_frame[image_idx].swapchain_release_semaphore == VK_NULL_HANDLE) {
    VkSemaphoreCreateInfo semaphore_ci = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    VkResult result = vkCreateSemaphore(gfx_state->device, &semaphore_ci, 0, &vkw->per_frame[image_idx].swapchain_release_semaphore);
    Assert(result == VK_SUCCESS);
  }

  VkPipelineStageFlags wait_stage = { VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT };
  VkSubmitInfo submit_info = {
    .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .waitSemaphoreCount   = 1,
    .pWaitSemaphores      = &vkw->per_frame[image_idx].swapchain_acquire_semaphore,
    .pWaitDstStageMask    = &wait_stage,
    .commandBufferCount   = 1,
    .pCommandBuffers      = &cmd,
    .signalSemaphoreCount = 1,
    .pSignalSemaphores    = &vkw->per_frame[image_idx].swapchain_release_semaphore,
  };
  VkResult result = vkQueueSubmit(gfx_state->queue, 1, &submit_info, vkw->per_frame[image_idx].queue_submit_fence);
  Assert(result == VK_SUCCESS);

  ////////////////////////////////
  //~ kti: Present

  VkPresentInfoKHR present_info = {
    .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores    = &vkw->per_frame[image_idx].swapchain_release_semaphore,
    .swapchainCount     = 1,
    .pSwapchains        = &vkw->swapchain,
    .pImageIndices      = &image_idx,
  };
  result = vkQueuePresentKHR(gfx_state->queue, &present_info);

  if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR) {
    gfx_vk_recreate_swapchain(os_window, vkw);
  } else if (result != VK_SUCCESS) {
    printf("Failed to present swapchain image.\n");
  }
}

#endif