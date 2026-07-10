#if (DEF_)
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
  X(vkCmdBindIndexBuffer) \
  X(vkCmdCopyBuffer) \
  X(vkCmdDraw) \
  X(vkCmdDrawIndexed) \
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
  X(vkGetDeviceProcAddr) \
  X(vkDestroySemaphore)

#define VK_EXTENSION_FUNCTIONS \
  X(vkCreateDebugReportCallbackEXT) \
  X(vkDestroyDebugReportCallbackEXT) \
  X(vkDebugReportMessageEXT)

#define X(name) Global PFN_##name name = 0;
VK_CORE_FUNCTIONS
VK_EXTENSION_FUNCTIONS
#undef X
#endif

// TODO(kti): Find a way to avoid this.
// Dynamically allocate new buffers if needed.
#define MAX_RECTANGLE_COUNT 8192
#define MAX_MESH_INSTANCE_COUNT 8192

#if (HEADER)

typedef struct GFX_VK_Buffer GFX_VK_Buffer;
struct GFX_VK_Buffer {
  VkBuffer buffer;
  VkDeviceMemory memory;
  void *ptr;
  VkDeviceSize size;
};

typedef struct GFX_Per_Frame GFX_Per_Frame;
struct GFX_Per_Frame {
  VkFence queue_submit_fence;
  VkCommandPool command_pool;
  VkCommandBuffer command_buffer;
  VkSemaphore swapchain_acquire_semaphore;
  VkSemaphore swapchain_release_semaphore;

  GFX_VK_Buffer instance_buffer;
  L1 rect_instances_count;

  GFX_VK_Buffer mesh_instance_buffer;
  L1 mesh_instance_count;
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
  I1 image_idx;
};

typedef enum GFX_Texture_Usage {
  GFX_TEXTURE_USAGE__DYNAMIC,
  GFX_TEXTURE_USAGE__STATIC,
} GFX_Texture_Usage;

typedef enum GFX_Buffer_Usage {
  GFX_BUFFER_USAGE__DYNAMIC,
  GFX_BUFFER_USAGE__STATIC,
} GFX_Buffer_Usage;

typedef enum GFX_Buffer_Kind {
  GFX_BUFFER_KIND__VERTEX,
  GFX_BUFFER_KIND__INDEX,
} GFX_Buffer_Kind;

typedef struct GFX_Texture GFX_Texture;
struct GFX_Texture {
  GFX_Texture *next;

  VkImage image;
  VkImageView image_view;
  VkDeviceMemory memory;
  I1 width;
  I1 height;

  GFX_VK_Buffer staging;
};

typedef struct GFX_Buffer GFX_Buffer;
struct GFX_Buffer {
  GFX_Buffer *next;

  GFX_Buffer_Kind kind;
  GFX_VK_Buffer main;
  GFX_VK_Buffer staging;
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

typedef struct GFX_Mesh_Vertex GFX_Mesh_Vertex;
struct GFX_Mesh_Vertex {
  F4 pos;
  F4 normal;
  F2 uv;
  F4 color;
};

typedef struct GFX_Mesh_Instance GFX_Mesh_Instance;
struct GFX_Mesh_Instance {
  M4F transform;
  F4 color;
};

typedef struct GFX_Rect_Batch GFX_Rect_Batch;
struct GFX_Rect_Batch {
  GFX_Rect_Batch *next;

  F4 clip_rect;
  GFX_Texture *texture;

  GFX_Rect_Instance *instances;
  L1 instance_cap;
  L1 instance_count;
};

typedef struct GFX_Mesh_Batch GFX_Mesh_Batch;
struct GFX_Mesh_Batch {
  GFX_Mesh_Batch *next;

  GFX_Buffer *vertex_buffer;
  L1 vertex_offset;
  L1 vertex_count;

  GFX_Buffer *index_buffer;
  L1 index_offset;
  L1 index_count;

  GFX_Mesh_Instance *instances;
  L1 instance_cap;
  L1 instance_count;
};

typedef struct GFX_Rect_Pass GFX_Rect_Pass;
struct GFX_Rect_Pass {
  GFX_Rect_Batch *first_batch;
  GFX_Rect_Batch *last_batch;
};

typedef struct GFX_Mesh_Pass GFX_Mesh_Pass;
struct GFX_Mesh_Pass {
  M4F view_projection;
  GFX_Mesh_Batch *first_batch;
  GFX_Mesh_Batch *last_batch;
};

typedef enum GFX_Pass_Kind  {
  GFX_PASS_KIND__RECT,
  GFX_PASS_KIND__MESH,
} GFX_Pass_Kind;

typedef struct GFX_Pass GFX_Pass;
struct GFX_Pass {
  GFX_Pass *next;
  GFX_Pass_Kind kind;

  union {
    GFX_Rect_Pass rect;
    GFX_Mesh_Pass mesh;
  };
};

typedef struct GFX_Pass_List GFX_Pass_List;
struct GFX_Pass_List {
  GFX_Pass *first;
  GFX_Pass *last;
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
  VkPipelineLayout mesh_pipeline_layout;
  VkPipeline mesh_pipeline;

  L1 recycle_semaphores_count;
  VkSemaphore recycle_semaphores[16];

  GFX_Window *first_free_window;
  GFX_Texture *first_free_texture;
  GFX_Buffer *first_free_buffer;

  VkSampler texture_sampler;
  VkDescriptorSetLayout descriptor_set_layout;
  VkCommandPool upload_command_pool;
  GFX_Texture *white_texture;
};

#endif

#if (SOURCE)

Global GFX_State *gfx_state = 0;
Global PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSetKHR = 0;

VKAPI_ATTR VkBool32 VKAPI_CALL debug_report_callback(VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT object_type, uint64_t object, size_t location,
    int32_t message_code, const char* layer_prefix, const char* message, void* user_data) {
  printf("%s - %s\n", layer_prefix, message);
  return VK_FALSE;
}

Internal VkDeviceMemory gfx_vk_allocate_memory(VkMemoryRequirements memory_requirements, VkMemoryPropertyFlags required_properties) {
  VkDeviceMemory memory = VK_NULL_HANDLE;

  VkPhysicalDeviceMemoryProperties mem_properties;
  vkGetPhysicalDeviceMemoryProperties(gfx_state->physical_device, &mem_properties);

  for (I1 i = 0; i < mem_properties.memoryTypeCount; i++) {
    if ((memory_requirements.memoryTypeBits & (1u << i)) &&
        (mem_properties.memoryTypes[i].propertyFlags & required_properties) == required_properties) {
      VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memory_requirements.size,
        .memoryTypeIndex = i,
      };
      VkDeviceMemory allocated_memory = VK_NULL_HANDLE;
      if (vkAllocateMemory(gfx_state->device, &alloc_info, 0, &allocated_memory) == VK_SUCCESS) {
        memory = allocated_memory;
      }
      break;
    }
  }

  return memory;
}

Internal void gfx_vk_destroy_buffer(GFX_VK_Buffer buffer);

Internal GFX_VK_Buffer gfx_vk_create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties) {
  GFX_VK_Buffer buffer = {.size = size};

  VkBufferCreateInfo buffer_ci = {
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .size = size,
    .usage = usage,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
  };
  vkCreateBuffer(gfx_state->device, &buffer_ci, 0, &buffer.buffer);

  VkMemoryRequirements memory_requirements;
  vkGetBufferMemoryRequirements(gfx_state->device, buffer.buffer, &memory_requirements);
  buffer.memory = gfx_vk_allocate_memory(memory_requirements, memory_properties);

  if (buffer.buffer != VK_NULL_HANDLE && buffer.memory != VK_NULL_HANDLE) {
    vkBindBufferMemory(gfx_state->device, buffer.buffer, buffer.memory, 0);
  }

  return buffer;
}

Internal GFX_VK_Buffer gfx_vk_create_mapped_buffer(VkDeviceSize size, VkBufferUsageFlags usage) {
  GFX_VK_Buffer buffer = gfx_vk_create_buffer( size, usage,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  if (buffer.memory != VK_NULL_HANDLE) {
    vkMapMemory(gfx_state->device, buffer.memory, 0, size, 0, &buffer.ptr);
  }

  return buffer;
}

Internal void gfx_vk_destroy_buffer(GFX_VK_Buffer buffer) {
  if (buffer.ptr != 0 && buffer.memory != VK_NULL_HANDLE) {
    vkUnmapMemory(gfx_state->device, buffer.memory);
  }
  if (buffer.buffer != VK_NULL_HANDLE) {
    vkDestroyBuffer(gfx_state->device, buffer.buffer, 0);
  }
  if (buffer.memory != VK_NULL_HANDLE) {
    vkFreeMemory(gfx_state->device, buffer.memory, 0);
  }
}

Internal VkCommandBuffer gfx_vk_immediate_begin(void) {
  VkCommandBuffer cmd = VK_NULL_HANDLE;
  VkCommandBufferAllocateInfo cmd_alloc_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .commandPool = gfx_state->upload_command_pool,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = 1,
  };
  vkAllocateCommandBuffers(gfx_state->device, &cmd_alloc_info, &cmd);

  if (cmd != VK_NULL_HANDLE) {
    VkCommandBufferBeginInfo begin_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vkBeginCommandBuffer(cmd, &begin_info);
  }

  return cmd;
}

Internal void gfx_vk_immediate_end(VkCommandBuffer cmd) {
  if (cmd != VK_NULL_HANDLE) {
    VkResult result = vkEndCommandBuffer(cmd);

    if (result == VK_SUCCESS) {
      VkSubmitInfo submit = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd,
      };
      vkQueueSubmit(gfx_state->queue, 1, &submit, VK_NULL_HANDLE);
      vkQueueWaitIdle(gfx_state->queue);
    }

    if (result != VK_ERROR_DEVICE_LOST) {
      vkResetCommandPool(gfx_state->device, gfx_state->upload_command_pool, 0);
    }
  }
}

Internal VkShaderModule gfx_vk_create_shader_module(String8 code) {
  VkShaderModule shader = VK_NULL_HANDLE;

  if (code.len > 0) {
    VkShaderModuleCreateInfo shader_ci = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = code.len,
      .pCode = (const uint32_t *)code.str,
    };
    vkCreateShaderModule(gfx_state->device, &shader_ci, 0, &shader);
  }

  return shader;
}

Internal void gfx_buffer_fill(GFX_Buffer *buffer, L1 offset, L1 size, void *data) {
  if (buffer != 0 && buffer->main.buffer != VK_NULL_HANDLE && buffer->main.memory != VK_NULL_HANDLE &&
    data != 0 && size > 0 && offset <= buffer->main.size && offset+size > offset && offset+size <= buffer->main.size) {

    //- kti: Get staging buffer.
    GFX_VK_Buffer staging = buffer->staging;
    I1 use_temp_staging = (staging.buffer == VK_NULL_HANDLE);
    if (use_temp_staging) {
      staging = gfx_vk_create_mapped_buffer(buffer->main.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    }

    if (staging.ptr != 0) {
      //- kti: Move data into staging buffer.
      memmove((B1 *)staging.ptr + offset, data, size);

      //- kti: Move data from staging buffer to final buffer. 
      VkCommandBuffer cmd = gfx_vk_immediate_begin();

      VkBufferCopy copy_region = {
        .srcOffset = offset,
        .dstOffset = offset,
        .size = size,
      };
      vkCmdCopyBuffer(cmd, staging.buffer, buffer->main.buffer, 1, &copy_region);

      VkAccessFlags2 dst_access = VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;
      if (buffer->kind == GFX_BUFFER_KIND__INDEX) {
        dst_access = VK_ACCESS_2_INDEX_READ_BIT;
      }
      VkBufferMemoryBarrier2 buffer_barrier = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
        .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT,
        .dstAccessMask = dst_access,
        .buffer = buffer->main.buffer,
        .offset = offset,
        .size = size,
      };
      VkDependencyInfo dependency_info = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .bufferMemoryBarrierCount = 1,
        .pBufferMemoryBarriers = &buffer_barrier,
      };
      vkCmdPipelineBarrier2(cmd, &dependency_info);

      gfx_vk_immediate_end(cmd);
    }

    //- kti: Cleanup temporary staging buffer.
    if (use_temp_staging) {
      gfx_vk_destroy_buffer(staging);
    }
  }
}

Internal GFX_Buffer *gfx_buffer_alloc(GFX_Buffer_Usage usage, GFX_Buffer_Kind kind, L1 size, void *initial_data) {
  GFX_Buffer *buffer = 0;

  if (size > 0) {
    buffer = gfx_state->first_free_buffer;
    if (buffer == 0) {
      buffer = push_array(gfx_state->arena, GFX_Buffer, 1);
    } else {
      SLLStackPop(gfx_state->first_free_buffer);
      MemoryZeroStruct(buffer);
    }

    VkBufferUsageFlags vk_usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    if (kind == GFX_BUFFER_KIND__VERTEX) {
      vk_usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    } else {
      vk_usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }
    buffer->main = gfx_vk_create_buffer(size, vk_usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    buffer->kind = kind;

    if (usage == GFX_BUFFER_USAGE__DYNAMIC) {
      buffer->staging = gfx_vk_create_mapped_buffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    }
    if (initial_data != 0) {
      gfx_buffer_fill(buffer, 0, size, initial_data);
    }
  }

  return buffer;
}

Internal void gfx_buffer_free(GFX_Buffer *buffer) {
  if (buffer != 0) {
    gfx_vk_destroy_buffer(buffer->main);
    gfx_vk_destroy_buffer(buffer->staging);

    //- kti: Add to freelist.
    MemoryZeroStruct(buffer);
    SLLStackPush(gfx_state->first_free_buffer, buffer);
  }
}

////////////////////////////////
//~ kti: Texture

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

Internal void gfx_vk_recycle_semaphore(VkSemaphore semaphore) {
  if (semaphore != VK_NULL_HANDLE) {
    if (gfx_state->recycle_semaphores_count < ArrayCount(gfx_state->recycle_semaphores)) {
      gfx_state->recycle_semaphores[gfx_state->recycle_semaphores_count] = semaphore;
      gfx_state->recycle_semaphores_count += 1;
    } else {
      vkDestroySemaphore(gfx_state->device, semaphore, 0);
    }
  }
}

Internal GFX_Texture *gfx_tex2d_alloc(GFX_Texture_Usage usage, I1 width, I1 height, void *pixels) {
  ProfFuncBegin();

  GFX_Texture *texture = gfx_state->first_free_texture;
  if (texture == 0) {
    texture = push_array(gfx_state->arena, GFX_Texture, 1);
  } else {
    SLLStackPop(gfx_state->first_free_texture);
  }
  MemoryZeroStruct(texture);

  L1 image_size = (L1)width * height * 4; // RGBA8

  //- kti: Create Image

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
  vkCreateImage(gfx_state->device, &image_ci, 0, &texture->image);

  VkMemoryRequirements image_mem_reqs = {0};
  vkGetImageMemoryRequirements(gfx_state->device, texture->image, &image_mem_reqs);
  texture->memory = gfx_vk_allocate_memory(image_mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  if (texture->image != VK_NULL_HANDLE && texture->memory != VK_NULL_HANDLE) {
    vkBindImageMemory(gfx_state->device, texture->image, texture->memory, 0);
  }

  //- kti: Create View
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
  if (texture->image != VK_NULL_HANDLE && texture->memory != VK_NULL_HANDLE) {
    vkCreateImageView(gfx_state->device, &view_ci, 0, &texture->image_view);
  }

  //- kti: Store dimensions
  if (texture->image_view != VK_NULL_HANDLE) {
    texture->width = width;
    texture->height = height;
  }

  ////////////////////////////////
  //~ kti: Upload


  if (texture->image_view != VK_NULL_HANDLE && (usage == GFX_TEXTURE_USAGE__DYNAMIC || pixels != 0)) {
    //- kti: Create staging buffer.
    GFX_VK_Buffer staging = gfx_vk_create_mapped_buffer(image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    VkCommandBuffer cmd = gfx_vk_immediate_begin();

    if (pixels != 0) {
      //- kti: Upload to staging buffer.
      memmove(staging.ptr, pixels, image_size);

      //- kti: Upload via command buffer.
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
      vkCmdCopyBufferToImage(cmd, staging.buffer, texture->image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

      gfx_vk_transition_image_layout(cmd, texture->image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_ACCESS_2_SHADER_READ_BIT,
        VK_PIPELINE_STAGE_2_TRANSFER_BIT,
        VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);
    } else {
      //- kti: Transition image.
      gfx_vk_transition_image_layout(cmd, texture->image,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        0, VK_ACCESS_2_SHADER_READ_BIT,
        VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);
    }

    gfx_vk_immediate_end(cmd);

    //- kti: For dynamic textures we keep the staging buffer.
    if (cmd != VK_NULL_HANDLE && usage == GFX_TEXTURE_USAGE__DYNAMIC) {
      texture->staging = staging;
    } else {
      gfx_vk_destroy_buffer(staging);
    }
  }

  ProfEnd();
  return texture;
}

Internal void gfx_fill_tex2d_region(GFX_Texture *tex, SI4 region, void *pixels) {
  if (tex != 0 &&
    region[0] < tex->width && region[1] < tex->height &&
    region[2] <= tex->width - region[0] && region[3] <= tex->height - region[1] &&
    region[2] > 0 && region[3] > 0) {
    ProfFuncBegin();

    L1 region_size = (L1)region[2] * region[3] * 4; // RGBA8

    //- kti: Use persistent staging buffer if available, otherwise create temporary.
    GFX_VK_Buffer staging = tex->staging;
    I1 use_temp_staging = (tex->staging.buffer == VK_NULL_HANDLE);
    if (use_temp_staging) {
      staging = gfx_vk_create_mapped_buffer(region_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    }

    if (staging.ptr != 0) {
      //- kti: Copy pixels to staging buffer.
      memmove(staging.ptr, pixels, region_size);

      //- kti: Upload to image.
      VkCommandBuffer cmd = gfx_vk_immediate_begin();

      gfx_vk_transition_image_layout(cmd, tex->image,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_ACCESS_2_SHADER_READ_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
        VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
        VK_PIPELINE_STAGE_2_TRANSFER_BIT);

      VkBufferImageCopy copy_region = {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = {
          .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
          .mipLevel = 0,
          .baseArrayLayer = 0,
          .layerCount = 1,
        },
        .imageOffset = {region[0], region[1], 0},
        .imageExtent = {region[2], region[3], 1},
      };
      vkCmdCopyBufferToImage(cmd, staging.buffer, tex->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

      gfx_vk_transition_image_layout(cmd, tex->image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_ACCESS_2_SHADER_READ_BIT,
        VK_PIPELINE_STAGE_2_TRANSFER_BIT,
        VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);

      gfx_vk_immediate_end(cmd);
    }

    //- kti: Cleanup temporary staging buffer.
    if (use_temp_staging) {
      gfx_vk_destroy_buffer(staging);
    }

    ProfEnd();
  }
}

Internal void gfx_tex2d_free(GFX_Texture *tex) {
  if (tex == 0 || tex->image == VK_NULL_HANDLE) {
    return;
  }

  vkDestroyImageView(gfx_state->device, tex->image_view, 0);
  vkDestroyImage(gfx_state->device, tex->image, 0);
  vkFreeMemory(gfx_state->device, tex->memory, 0);

  //- kti: Cleanup staging buffer for dynamic textures.
  gfx_vk_destroy_buffer(tex->staging);

  MemoryZeroStruct(tex);
  SLLStackPush(gfx_state->first_free_texture, tex);
}

Internal void gfx_init() {
  Arena *arena = arena_alloc(MiB(64));
  gfx_state = push_array(arena, GFX_State, 1);
  gfx_state->arena = arena;

  ////////////////////////////////
  //~ kti: Load Core Procedures

  void *lib = os_library_open(str8("libvulkan.so.1"));
  Assert(lib != 0);

#define X(name) *(void **)&name = os_library_load_proc(lib, str8(#name));
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
  for (L1 i = 0; i < layer_count; i += 1) {
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
  for (L1 i = 0; i < extension_count; i += 1) {
    for (L1 j = 0; j < ArrayCount(required_extensions); j += 1) {
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

  for (L1 i = 0; i < device_count; i += 1) {
    VkPhysicalDeviceProperties props = {0};
    vkGetPhysicalDeviceProperties(physical_devices[i], &props);

    I1 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_devices[i], &queue_family_count, 0);
    VkQueueFamilyProperties *queue_family_properties = push_array(arena, VkQueueFamilyProperties, queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_devices[i], &queue_family_count, queue_family_properties);

    for (L1 j = 0; j < queue_family_count; j += 1) {
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

  VkShaderModule vert_shader = gfx_vk_create_shader_module(
    os_read_entire_file(arena, str8("./shaders/shader.vert.spv")));
  VkShaderModule frag_shader = gfx_vk_create_shader_module(
    os_read_entire_file(arena, str8("./shaders/shader.frag.spv")));

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
  //~ kti: Mesh Pipeline

  VkPushConstantRange mesh_push_constants_range = {
    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
    .offset = 0,
    .size = sizeof(M4F),
  };

  VkPipelineLayoutCreateInfo mesh_layout_ci = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .pushConstantRangeCount = 1,
    .pPushConstantRanges = &mesh_push_constants_range,
  };
  result = vkCreatePipelineLayout(gfx_state->device, &mesh_layout_ci, 0, &gfx_state->mesh_pipeline_layout);
  Assert(result == VK_SUCCESS);

  VkVertexInputBindingDescription mesh_binding_descriptions[] = {
    {
      .binding = 0,
      .stride = sizeof(GFX_Mesh_Vertex),
      .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    },
    {
      .binding = 1,
      .stride = sizeof(GFX_Mesh_Instance),
      .inputRate = VK_VERTEX_INPUT_RATE_INSTANCE,
    },
  };

  VkVertexInputAttributeDescription mesh_attribute_descriptions[] = {
    {.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(GFX_Mesh_Vertex, pos)},
    {.location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(GFX_Mesh_Vertex, normal)},
    {.location = 2, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT,       .offset = offsetof(GFX_Mesh_Vertex, uv)},
    {.location = 3, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(GFX_Mesh_Vertex, color)},
    {.location = 4, .binding = 1, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(GFX_Mesh_Instance, transform.c[0])},
    {.location = 5, .binding = 1, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(GFX_Mesh_Instance, transform.c[1])},
    {.location = 6, .binding = 1, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(GFX_Mesh_Instance, transform.c[2])},
    {.location = 7, .binding = 1, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(GFX_Mesh_Instance, transform.c[3])},
    {.location = 8, .binding = 1, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(GFX_Mesh_Instance, color)},
  };

  VkPipelineVertexInputStateCreateInfo mesh_vertex_input = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .vertexBindingDescriptionCount = ArrayCount(mesh_binding_descriptions),
    .pVertexBindingDescriptions = mesh_binding_descriptions,
    .vertexAttributeDescriptionCount = ArrayCount(mesh_attribute_descriptions),
    .pVertexAttributeDescriptions = mesh_attribute_descriptions,
  };

  VkShaderModule mesh_vert_shader = gfx_vk_create_shader_module(
    os_read_entire_file(arena, str8("./shaders/mesh.vert.spv")));
  VkShaderModule mesh_frag_shader = gfx_vk_create_shader_module(
    os_read_entire_file(arena, str8("./shaders/mesh.frag.spv")));

  VkPipelineShaderStageCreateInfo mesh_shader_stages[] = {
    {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_VERTEX_BIT,
      .module = mesh_vert_shader,
      .pName = "main",
    },
    {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
      .module = mesh_frag_shader,
      .pName = "main",
    },
  };

  VkGraphicsPipelineCreateInfo mesh_pipeline_ci = {
    .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .pNext               = &pipeline_rendering,
    .stageCount          = ArrayCount(mesh_shader_stages),
    .pStages             = mesh_shader_stages,
    .pVertexInputState   = &mesh_vertex_input,
    .pInputAssemblyState = &input_assembly,
    .pViewportState      = &viewport,
    .pRasterizationState = &raster,
    .pMultisampleState   = &multisample,
    .pDepthStencilState  = &depth_stencil,
    .pColorBlendState    = &blend,
    .pDynamicState       = &dynamic_state_ci,
    .layout              = gfx_state->mesh_pipeline_layout,
    .renderPass          = VK_NULL_HANDLE,
    .subpass             = 0,
  };

  result = vkCreateGraphicsPipelines(gfx_state->device, VK_NULL_HANDLE, 1, &mesh_pipeline_ci, 0, &gfx_state->mesh_pipeline);
  Assert(result == VK_SUCCESS);

  vkDestroyShaderModule(gfx_state->device, mesh_vert_shader, 0);
  vkDestroyShaderModule(gfx_state->device, mesh_frag_shader, 0);

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
  gfx_state->white_texture = gfx_tex2d_alloc(GFX_TEXTURE_USAGE__STATIC, 1, 1, &white);
}

Internal void gfx_vk_recreate_swapchain(OS_Window *os_window, GFX_Window *vkw) {
  vkDeviceWaitIdle(gfx_state->device);

  //- kti: Destroy old resources (if they exist).
  if (vkw->swapchain != VK_NULL_HANDLE) {
    vkDestroySwapchainKHR(gfx_state->device, vkw->swapchain, 0);
    vkw->swapchain = VK_NULL_HANDLE;
  }
  for (L1 i = 0; i < vkw->image_count; i += 1) {
    vkDestroyImageView(gfx_state->device, vkw->swapchain_image_views[i], 0);
  }

  //- kti: Our pipeline uses BGRA_SRGB so we force it. Not optimal.
  VkFormat color_format = VK_FORMAT_UNDEFINED;

  if (vkw->surface_format_count == 1 || vkw->surface_formats[0].format == VK_FORMAT_UNDEFINED) {
    color_format = VK_FORMAT_B8G8R8A8_SRGB;
  } else {
    for (L1 i = 0; i < vkw->surface_format_count; i += 1) {
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
  for (L1 i = 0; i < vkw->present_mode_count; i += 1) {
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

  for (L1 i = 0; i < vkw->image_count; i += 1) {
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
  vkw->image_idx = I1_MAX;

  ////////////////////////////////
  //~ kti: Surface

  VkWaylandSurfaceCreateInfoKHR surface_ci = {
    .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
    .display = os_gfx_state->display,
    .surface = window->surface,
  };
  VkResult result = vkCreateWaylandSurfaceKHR(gfx_state->instance, &surface_ci, 0, &vkw->surface);
  Assert(result == VK_SUCCESS);

  VkBool32 supports = vkGetPhysicalDeviceWaylandPresentationSupportKHR(gfx_state->physical_device, gfx_state->present_queue_index, os_gfx_state->display);
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

  for (L1 i = 0; i < vkw->image_count; i += 1) {
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
    frame->instance_buffer = gfx_vk_create_mapped_buffer(
      sizeof(GFX_Rect_Instance) * MAX_RECTANGLE_COUNT,
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    //- kti: Mesh Instance Stream Buffer
    frame->mesh_instance_buffer = gfx_vk_create_mapped_buffer(
      sizeof(GFX_Mesh_Instance) * MAX_MESH_INSTANCE_COUNT,
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  }

  return vkw;
}

Internal void gfx_window_unequip(GFX_Window *vkw) {
  ////////////////////////////////
  //~ kti: Destroy Per frame Resources

  //- kti: Wait for all GPU work to finish (including presents).
  vkQueueWaitIdle(gfx_state->queue);

  //- kti: Free per frame resources.
  for (L1 i = 0; i < vkw->image_count; i += 1) {
    GFX_Per_Frame *frame = &vkw->per_frame[i];

    if (frame->queue_submit_fence != VK_NULL_HANDLE) {
      vkDestroyFence(gfx_state->device, frame->queue_submit_fence, 0);
    }

    if (frame->command_pool != VK_NULL_HANDLE) {
      vkDestroyCommandPool(gfx_state->device, frame->command_pool, 0);
    }

    vkDestroyImageView(gfx_state->device, vkw->swapchain_image_views[i], 0);

    //- kti: Free instance buffers.
    gfx_vk_destroy_buffer(frame->instance_buffer);
    gfx_vk_destroy_buffer(frame->mesh_instance_buffer);

    //- kti: Recycle semaphores.
    gfx_vk_recycle_semaphore(frame->swapchain_acquire_semaphore);
    gfx_vk_recycle_semaphore(frame->swapchain_release_semaphore);
  }

  ////////////////////////////////
  //~ kti: Destroy Per Window Resources

  vkDestroySwapchainKHR(gfx_state->device, vkw->swapchain, 0);
  vkDestroySurfaceKHR(gfx_state->instance, vkw->surface, 0);

  SLLStackPush(gfx_state->first_free_window, vkw);
}

Internal void gfx_window_begin_frame(OS_Window *os_window, GFX_Window *vkw) {
  ProfFuncBegin();

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
    gfx_state->recycle_semaphores_count -= 1;
    acquire_semaphore = gfx_state->recycle_semaphores[gfx_state->recycle_semaphores_count];
  }

  //- kti: Grab the next image.
  I1 image_idx = 0;
  VkResult img_acquire_result = VK_RESULT_MAX_ENUM;
  for (L1 try = 0; try < 2; try += 1) {
    img_acquire_result = vkAcquireNextImageKHR(gfx_state->device, vkw->swapchain, L1_MAX, acquire_semaphore, VK_NULL_HANDLE, &image_idx);
    if (img_acquire_result == VK_SUBOPTIMAL_KHR || img_acquire_result == VK_ERROR_OUT_OF_DATE_KHR) {
      gfx_vk_recreate_swapchain(os_window, vkw);
    } else {
      break;
    }
  }


  //- kti: If grabbing image failed we return early.
  if (img_acquire_result != VK_SUCCESS) {
    // - kti: Recycle the semaphore we never used.
    gfx_vk_recycle_semaphore(acquire_semaphore);
    vkQueueWaitIdle(gfx_state->queue);
    image_idx = I1_MAX;
  } else {
    //- kti: Wait for previous work submitted to this image is complete.
    vkWaitForFences(gfx_state->device, 1, &vkw->per_frame[image_idx].queue_submit_fence, VK_TRUE, L1_MAX);
    vkResetFences(gfx_state->device, 1, &vkw->per_frame[image_idx].queue_submit_fence);

    //- kti: Recycle old semaphore and store new one.
    gfx_vk_recycle_semaphore(vkw->per_frame[image_idx].swapchain_acquire_semaphore);
    vkw->per_frame[image_idx].swapchain_acquire_semaphore = acquire_semaphore;

    // TODO(kti): Look into whether or not we need to release resources.
    vkResetCommandPool(gfx_state->device, vkw->per_frame[image_idx].command_pool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);

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
    vkw->per_frame[image_idx].mesh_instance_count = 0;
  }

  vkw->image_idx = image_idx;

  ProfEnd();
}

Internal void gfx_window_submit(OS_Window *os_window, GFX_Window *vkw, GFX_Pass_List passes) {
  I1 image_idx = vkw->image_idx;
  if (image_idx < vkw->image_count) {
    GFX_Per_Frame *frame = &vkw->per_frame[image_idx];
    VkCommandBuffer cmd = frame->command_buffer;
    GFX_Rect_Instance *rect_instances = (GFX_Rect_Instance *)frame->instance_buffer.ptr;
    L1 rect_instances_count = frame->rect_instances_count;
    GFX_Mesh_Instance *mesh_instances = (GFX_Mesh_Instance *)frame->mesh_instance_buffer.ptr;
    L1 mesh_instance_count = frame->mesh_instance_count;

    for (GFX_Pass *pass = passes.first; pass != 0; pass = pass->next) {
      if (pass->kind == GFX_PASS_KIND__RECT) {
        VkDeviceSize offset = {0};
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, gfx_state->pipeline);
        vkCmdBindVertexBuffers(cmd, 0, 1, &frame->instance_buffer.buffer, &offset);

        GFX_Rect_Pass rect_pass = pass->rect;
        for (GFX_Rect_Batch *batch = rect_pass.first_batch; batch != 0; batch = batch->next) {
          // TODO(kti): For now we skip batches that would exceed the max count and continue, in case the next one doesn't. We could also clip the batch and break.
          // Alternatively we could allocate a buffer that could contain the entire batch. This would allow for "infinite" rectangle counts.
          if (rect_instances_count + batch->instance_count > MAX_RECTANGLE_COUNT) {
            printf("Max number of rectangle instances reached for frame. Skipping batch.\n");
            continue;
          }

          //- kti: Clip
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
          if (batch->clip_rect[0] != 0.0f || batch->clip_rect[1] != 0.0f ||
              batch->clip_rect[2] > 0.0f || batch->clip_rect[3] > 0.0f) {
            F4 screen_rect = {0, 0, vkw->swapchain_extent.width, vkw->swapchain_extent.height};
            F4 intersection = rect_overlap(screen_rect, batch->clip_rect);
            if (intersection[2] >= 0.0f && intersection[3] >= 0.0f) {
              scissor.offset.x = intersection[0];
              scissor.offset.y = intersection[1];
              scissor.extent.width = intersection[2];
              scissor.extent.height = intersection[3];
            } else {
              // NOTE(kti): Clip and screen rect don't overlap, we can skip the batch.
              continue;
            }
          }
          vkCmdSetScissor(cmd, 0, 1, &scissor);

          //- kti: Texture
          GFX_Texture *texture = batch->texture;
          if (texture == 0) {
            texture = gfx_state->white_texture;
          }
          if (texture == 0 || texture->image_view == VK_NULL_HANDLE) {
            continue;
          }
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

          //- kti: Push Constants
          F1 push_data[4] = {
            vkw->swapchain_extent.width, vkw->swapchain_extent.height,
            (F1)texture->width, (F1)texture->height
          };
          vkCmdPushConstants(cmd, gfx_state->pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(push_data), push_data);

          //- kti: Draw
          memmove(rect_instances + rect_instances_count, batch->instances, batch->instance_count*sizeof(GFX_Rect_Instance));
          vkCmdDraw(cmd, 6, batch->instance_count, 0, rect_instances_count);

          rect_instances_count += batch->instance_count;
        }
      } else if (pass->kind == GFX_PASS_KIND__MESH) {
        GFX_Mesh_Pass mesh_pass = pass->mesh;

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

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, gfx_state->mesh_pipeline);
        vkCmdSetScissor(cmd, 0, 1, &scissor);
        vkCmdPushConstants(cmd, gfx_state->mesh_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(M4F), &mesh_pass.view_projection);

        for (GFX_Mesh_Batch *batch = mesh_pass.first_batch; batch != 0; batch = batch->next) {
          if (batch->vertex_buffer == 0 ||
              batch->vertex_buffer->main.buffer == VK_NULL_HANDLE ||
              batch->vertex_buffer->kind != GFX_BUFFER_KIND__VERTEX ||
              batch->index_buffer == 0 ||
              batch->index_buffer->main.buffer == VK_NULL_HANDLE ||
              batch->index_buffer->kind != GFX_BUFFER_KIND__INDEX ||
              batch->vertex_count == 0 ||
              batch->index_count == 0 ||
              batch->instance_count == 0 ||
              batch->vertex_offset % sizeof(GFX_Mesh_Vertex) != 0 ||
              batch->index_offset % sizeof(I1) != 0 ||
              batch->index_count > I1_MAX ||
              batch->vertex_offset > batch->vertex_buffer->main.size ||
              batch->vertex_count > (batch->vertex_buffer->main.size - batch->vertex_offset) / sizeof(GFX_Mesh_Vertex) ||
              batch->index_offset > batch->index_buffer->main.size ||
              batch->index_count > (batch->index_buffer->main.size - batch->index_offset) / sizeof(I1)) {
            continue;
          }

          if (mesh_instance_count + batch->instance_count > MAX_MESH_INSTANCE_COUNT) {
            printf("Max mesh instance count reached for frame. Skipping batch.\n");
            continue;
          }

          memmove(mesh_instances + mesh_instance_count, batch->instances, batch->instance_count*sizeof(GFX_Mesh_Instance));

          VkBuffer vertex_buffers[] = {
            batch->vertex_buffer->main.buffer,
            frame->mesh_instance_buffer.buffer,
          };
          VkDeviceSize vertex_offsets[] = {
            batch->vertex_offset,
            mesh_instance_count * sizeof(GFX_Mesh_Instance),
          };
          vkCmdBindVertexBuffers(cmd, 0, ArrayCount(vertex_buffers), vertex_buffers, vertex_offsets);
          vkCmdBindIndexBuffer(cmd, batch->index_buffer->main.buffer, batch->index_offset, VK_INDEX_TYPE_UINT32);

          vkCmdDrawIndexed(cmd, batch->index_count, batch->instance_count, 0, 0, 0);

          mesh_instance_count += batch->instance_count;
        }
      }
    }

    frame->rect_instances_count = rect_instances_count;
    frame->mesh_instance_count = mesh_instance_count;
  }
}

Internal void gfx_window_end_frame(OS_Window *os_window, GFX_Window *vkw) {
  ProfFuncBegin();

  I1 image_idx = vkw->image_idx;
  if (image_idx < vkw->image_count) {
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

  ProfEnd();
}

#endif
