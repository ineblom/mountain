#ifndef GFX_H
#define GFX_H
#define VK_NO_PROTOTYPES
#if defined(__APPLE__)
#define VK_USE_PLATFORM_METAL_EXT
#else
#define VK_USE_PLATFORM_WAYLAND_KHR
#endif
#include <vulkan/vulkan.h>

#if defined(__APPLE__)
#define GFX_VK_PLATFORM_FUNCTIONS X(vkCreateMetalSurfaceEXT)
#else
#define GFX_VK_PLATFORM_FUNCTIONS X(vkCreateWaylandSurfaceKHR)
#endif

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
  X(vkCmdPushConstants) \
  X(vkCreateImage) \
  X(vkGetImageMemoryRequirements) \
  X(vkBindImageMemory) \
  X(vkCreateSampler) \
  X(vkCmdCopyBufferToImage) \
  X(vkCmdClearAttachments) \
  X(vkCreateDescriptorSetLayout) \
  X(vkDestroyImage) \
  X(vkDestroySampler) \
  X(vkDestroyDescriptorSetLayout) \
  X(vkDestroyBuffer) \
  X(vkDestroyCommandPool) \
  X(vkFreeMemory) \
  X(vkDestroyFence) \
  X(vkGetDeviceProcAddr) \
  X(vkDestroySemaphore) \
  GFX_VK_PLATFORM_FUNCTIONS

#define VK_EXTENSION_FUNCTIONS \
  X(vkCreateDebugReportCallbackEXT) \
  X(vkDestroyDebugReportCallbackEXT) \
  X(vkDebugReportMessageEXT)

// TODO(kti): Find a way to avoid this.
// Dynamically allocate new buffers if needed.
#define MAX_RECTANGLE_COUNT 8192
#define MAX_MESH_INSTANCE_COUNT 8192

typedef struct GFX_VK_Buffer GFX_VK_Buffer;
struct GFX_VK_Buffer {
  VkBuffer buffer;
  VkDeviceMemory memory;
  void *ptr;
  VkDeviceSize size;
};

typedef struct GFX_VK_Image GFX_VK_Image;
struct GFX_VK_Image {
  VkImage image;
  VkImageView view;
  VkDeviceMemory memory;
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

  GFX_VK_Image depth;

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

  GFX_VK_Image image;
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
};

typedef I1 GFX_Mesh_Feature_Flags;
enum {
  GFX_MESH_FEATURE__NONE = 0,
  GFX_MESH_FEATURE__UNLIT = 1 << 0,
};

typedef struct GFX_Mesh_Instance GFX_Mesh_Instance;
struct GFX_Mesh_Instance {
  M4F transform;
  F4 color;
  GFX_Mesh_Feature_Flags feature_flags;
};

typedef struct GFX_Mesh_Push_Constants GFX_Mesh_Push_Constants;
struct GFX_Mesh_Push_Constants {
  M4F view_projection;
  F2 viewport_size;
  F1 outline_width;
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

  F1 outline_width;

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
  F4 viewport_rect;
  GFX_Mesh_Batch *first_batch;
  GFX_Mesh_Batch *last_batch;
};

typedef struct GFX_Clear_Depth_Pass GFX_Clear_Depth_Pass;
struct GFX_Clear_Depth_Pass {
  F4 rect;
  F1 depth;
};

typedef enum GFX_Pass_Kind  {
  GFX_PASS_KIND__RECT,
  GFX_PASS_KIND__MESH,
  GFX_PASS_KIND__MESH_OUTLINE,
  GFX_PASS_KIND__CLEAR_DEPTH,
} GFX_Pass_Kind;

typedef struct GFX_Pass GFX_Pass;
struct GFX_Pass {
  GFX_Pass *next;
  GFX_Pass_Kind kind;

  union {
    GFX_Rect_Pass rect;
    GFX_Mesh_Pass mesh;
    GFX_Clear_Depth_Pass clear_depth;
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
  VkPipeline mesh_outline_mask_pipeline;
  VkPipeline mesh_outline_pipeline;

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
