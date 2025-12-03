#if (CPU_ && DEF_)
#define VK_NO_PROTOTYPES
#define VK_USE_PLATFORM_WAYLAND_KHR
#include "vulkan.h"
#endif

#if (CPU_ && TYP_)

typedef struct GFX_Per_Frame GFX_Per_Frame;
struct GFX_Per_Frame {
  VkFence queue_submit_fence;
  VkCommandPool command_pool;
  VkCommandBuffer command_buffer;
  VkSemaphore swapchain_acquire_semaphore;
  VkSemaphore swapchain_release_semaphore;
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

typedef struct GFX_Vertex GFX_Vertex;
struct GFX_Vertex {
  F1 x, y;
  F1 r, g, b;
};

#endif

#if (CPU_ && RAM_)

VkInstance instance;
VkDebugReportCallbackEXT vk_debug_callback;
VkPhysicalDevice physical_device;
VkPhysicalDeviceProperties physical_devce_properties;
VkDevice device;
VkQueue queue;
L1 present_queue_index;
VkPipelineLayout pipeline_layout;
VkPipeline pipeline;

VkBuffer vertex_buffer;
VkDeviceMemory vertex_buffer_memory;

L1 recycle_semaphores_count;
VkSemaphore recycle_semaphores[16];

GFX_Window *first_free_vk_window;

#endif

#if (CPU_ && ROM_)

// Should every subsystem have its own arena. And not use the RAM system?
// Instead we have a global gfx_state and the first time we call gfx_init it allocs.

Global PFN_vkCreateInstance vkCreateInstance = 0;
Global PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties = 0;
Global PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties = 0;
Global PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = 0;
Global PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices = 0;
Global PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties = 0;
Global PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties = 0;
Global PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR = 0;
Global PFN_vkCreateDevice vkCreateDevice = 0;
Global PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR = 0;
Global PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR = 0;
Global PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR = 0;
Global PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR = 0;
Global PFN_vkCreateCommandPool vkCreateCommandPool = 0;
Global PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers = 0;
Global PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR = 0;
Global PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR = 0;
Global PFN_vkBeginCommandBuffer vkBeginCommandBuffer = 0;
Global PFN_vkEndCommandBuffer vkEndCommandBuffer = 0;
Global PFN_vkQueueSubmit vkQueueSubmit = 0;
Global PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR = 0;
Global PFN_vkCreateFence vkCreateFence = 0;
Global PFN_vkCmdPipelineBarrier2 vkCmdPipelineBarrier2 = 0;
Global PFN_vkWaitForFences vkWaitForFences = 0;
Global PFN_vkResetFences vkResetFences = 0;
Global PFN_vkResetCommandPool vkResetCommandPool = 0;
Global PFN_vkCreateImageView vkCreateImageView = 0;
Global PFN_vkGetPhysicalDeviceFeatures2 vkGetPhysicalDeviceFeatures2 = 0;
Global PFN_vkGetDeviceQueue vkGetDeviceQueue = 0;
Global PFN_vkCreatePipelineLayout vkCreatePipelineLayout = 0;
Global PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines = 0;
Global PFN_vkDestroyShaderModule vkDestroyShaderModule = 0;
Global PFN_vkCreateShaderModule vkCreateShaderModule = 0;
Global PFN_vkCreateSemaphore vkCreateSemaphore = 0;
Global PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR = 0;
Global PFN_vkQueuePresentKHR vkQueuePresentKHR = 0;
Global PFN_vkCmdBeginRendering vkCmdBeginRendering = 0;
Global PFN_vkCmdSetViewport vkCmdSetViewport = 0;
Global PFN_vkCmdBindPipeline vkCmdBindPipeline = 0;
Global PFN_vkCmdSetCullMode vkCmdSetCullMode = 0;
Global PFN_vkCmdSetFrontFace vkCmdSetFrontFace = 0;
Global PFN_vkCmdSetPrimitiveTopology vkCmdSetPrimitiveTopology = 0;
Global PFN_vkCmdEndRendering vkCmdEndRendering = 0;
Global PFN_vkCmdSetScissor vkCmdSetScissor = 0;
Global PFN_vkDestroyImageView vkDestroyImageView = 0;
Global PFN_vkDeviceWaitIdle vkDeviceWaitIdle = 0;
Global PFN_vkQueueWaitIdle vkQueueWaitIdle = 0;
Global PFN_vkCreateBuffer vkCreateBuffer = 0;
Global PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements = 0;
Global PFN_vkAllocateMemory vkAllocateMemory = 0;
Global PFN_vkMapMemory vkMapMemory = 0;
Global PFN_vkUnmapMemory vkUnmapMemory = 0;
Global PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties = 0;
Global PFN_vkBindBufferMemory vkBindBufferMemory = 0;
Global PFN_vkCmdBindVertexBuffers vkCmdBindVertexBuffers = 0;
Global PFN_vkCmdDraw vkCmdDraw = 0;

Global PFN_vkCreateWaylandSurfaceKHR vkCreateWaylandSurfaceKHR = 0;
Global PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR vkGetPhysicalDeviceWaylandPresentationSupportKHR = 0;

Global PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT = 0;
Global PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT = 0;
Global PFN_vkDebugReportMessageEXT vkDebugReportMessageEXT = 0;

VKAPI_ATTR VkBool32 VKAPI_CALL debug_report_callback(VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT object_type, uint64_t object, size_t location,
    int32_t message_code, const char* layer_prefix, const char* message, void* user_data) {
  printf("%s - %s\n", layer_prefix, message);
  return VK_FALSE;
}

Internal void gfx_init(Arena *arena) {

  ////////////////////////////////
  //~ kti: Load Core Procedures

  void *lib = os_library_open(Str8_("libvulkan.so.1"));
  Assert(lib != 0);

  *(void **)&vkCreateInstance = os_library_load_proc(lib, Str8_("vkCreateInstance"));
  *(void **)&vkEnumerateInstanceLayerProperties = os_library_load_proc(lib, Str8_("vkEnumerateInstanceLayerProperties"));
  *(void **)&vkEnumerateInstanceExtensionProperties = os_library_load_proc(lib, Str8_("vkEnumerateInstanceExtensionProperties"));
  *(void **)&vkGetInstanceProcAddr = os_library_load_proc(lib, Str8_("vkGetInstanceProcAddr"));
  *(void **)&vkEnumeratePhysicalDevices = os_library_load_proc(lib, Str8_("vkEnumeratePhysicalDevices"));
  *(void **)&vkGetPhysicalDeviceProperties = os_library_load_proc(lib, Str8_("vkGetPhysicalDeviceProperties"));
  *(void **)&vkGetPhysicalDeviceQueueFamilyProperties = os_library_load_proc(lib, Str8_("vkGetPhysicalDeviceQueueFamilyProperties"));
  *(void **)&vkGetPhysicalDeviceSurfaceSupportKHR = os_library_load_proc(lib, Str8_("vkGetPhysicalDeviceSurfaceSupportKHR"));
  *(void **)&vkCreateDevice = os_library_load_proc(lib, Str8_("vkCreateDevice"));
  *(void **)&vkGetPhysicalDeviceSurfaceFormatsKHR = os_library_load_proc(lib, Str8_("vkGetPhysicalDeviceSurfaceFormatsKHR"));
  *(void **)&vkGetPhysicalDeviceSurfaceCapabilitiesKHR = os_library_load_proc(lib, Str8_("vkGetPhysicalDeviceSurfaceCapabilitiesKHR"));
  *(void **)&vkGetPhysicalDeviceSurfacePresentModesKHR = os_library_load_proc(lib, Str8_("vkGetPhysicalDeviceSurfacePresentModesKHR"));
  *(void **)&vkCreateSwapchainKHR = os_library_load_proc(lib, Str8_("vkCreateSwapchainKHR"));
  *(void **)&vkCreateCommandPool = os_library_load_proc(lib, Str8_("vkCreateCommandPool"));
  *(void **)&vkAllocateCommandBuffers = os_library_load_proc(lib, Str8_("vkAllocateCommandBuffers"));
  *(void **)&vkDestroySwapchainKHR = os_library_load_proc(lib, Str8_("vkDestroySwapchainKHR"));
  *(void **)&vkDestroySurfaceKHR = os_library_load_proc(lib, Str8_("vkDestroySurfaceKHR"));
  *(void **)&vkBeginCommandBuffer = os_library_load_proc(lib, Str8_("vkBeginCommandBuffer"));
  *(void **)&vkEndCommandBuffer = os_library_load_proc(lib, Str8_("vkEndCommandBuffer"));
  *(void **)&vkQueueSubmit = os_library_load_proc(lib, Str8_("vkQueueSubmit"));
  *(void **)&vkGetSwapchainImagesKHR = os_library_load_proc(lib, Str8_("vkGetSwapchainImagesKHR"));
  *(void **)&vkCreateFence = os_library_load_proc(lib, Str8_("vkCreateFence"));
  *(void **)&vkCmdPipelineBarrier2 = os_library_load_proc(lib, Str8_("vkCmdPipelineBarrier2"));
  *(void **)&vkWaitForFences = os_library_load_proc(lib, Str8_("vkWaitForFences"));
  *(void **)&vkResetFences = os_library_load_proc(lib, Str8_("vkResetFences"));
  *(void **)&vkResetCommandPool = os_library_load_proc(lib, Str8_("vkResetCommandPool"));
  *(void **)&vkCreateImageView = os_library_load_proc(lib, Str8_("vkCreateImageView"));
  *(void **)&vkGetPhysicalDeviceFeatures2 = os_library_load_proc(lib, Str8_("vkGetPhysicalDeviceFeatures2"));
  *(void **)&vkGetDeviceQueue = os_library_load_proc(lib, Str8_("vkGetDeviceQueue"));
  *(void **)&vkCreatePipelineLayout = os_library_load_proc(lib, Str8_("vkCreatePipelineLayout"));
  *(void **)&vkCreateGraphicsPipelines = os_library_load_proc(lib, Str8_("vkCreateGraphicsPipelines"));
  *(void **)&vkDestroyShaderModule = os_library_load_proc(lib, Str8_("vkDestroyShaderModule"));
  *(void **)&vkCreateShaderModule = os_library_load_proc(lib, Str8_("vkCreateShaderModule"));
  *(void **)&vkCreateSemaphore = os_library_load_proc(lib, Str8_("vkCreateSemaphore"));
  *(void **)&vkAcquireNextImageKHR = os_library_load_proc(lib, Str8_("vkAcquireNextImageKHR"));
  *(void **)&vkQueuePresentKHR = os_library_load_proc(lib, Str8_("vkQueuePresentKHR"));
  *(void **)&vkCmdBeginRendering = os_library_load_proc(lib, Str8_("vkCmdBeginRendering"));
  *(void **)&vkCmdSetViewport = os_library_load_proc(lib, Str8_("vkCmdSetViewport"));
  *(void **)&vkCmdBindPipeline = os_library_load_proc(lib, Str8_("vkCmdBindPipeline"));
  *(void **)&vkCmdSetCullMode = os_library_load_proc(lib, Str8_("vkCmdSetCullMode"));
  *(void **)&vkCmdSetFrontFace = os_library_load_proc(lib, Str8_("vkCmdSetFrontFace"));
  *(void **)&vkCmdSetPrimitiveTopology = os_library_load_proc(lib, Str8_("vkCmdSetPrimitiveTopology"));
  *(void **)&vkCmdEndRendering = os_library_load_proc(lib, Str8_("vkCmdEndRendering"));
  *(void **)&vkCmdSetScissor = os_library_load_proc(lib, Str8_("vkCmdSetScissor"));
  *(void **)&vkDestroyImageView = os_library_load_proc(lib, Str8_("vkDestroyImageView"));
  *(void **)&vkDeviceWaitIdle = os_library_load_proc(lib, Str8_("vkDeviceWaitIdle"));
  *(void **)&vkQueueWaitIdle = os_library_load_proc(lib, Str8_("vkQueueWaitIdle"));
  *(void **)&vkCreateBuffer = os_library_load_proc(lib, Str8_("vkCreateBuffer"));
  *(void **)&vkGetBufferMemoryRequirements = os_library_load_proc(lib, Str8_("vkGetBufferMemoryRequirements"));
  *(void **)&vkAllocateMemory = os_library_load_proc(lib, Str8_("vkAllocateMemory"));
  *(void **)&vkMapMemory = os_library_load_proc(lib, Str8_("vkMapMemory"));
  *(void **)&vkUnmapMemory = os_library_load_proc(lib, Str8_("vkUnmapMemory"));
  *(void **)&vkGetPhysicalDeviceMemoryProperties = os_library_load_proc(lib, Str8_("vkGetPhysicalDeviceMemoryProperties"));
  *(void **)&vkBindBufferMemory = os_library_load_proc(lib, Str8_("vkBindBufferMemory"));
  *(void **)&vkCmdBindVertexBuffers = os_library_load_proc(lib, Str8_("vkCmdBindVertexBuffers"));
  *(void **)&vkCmdDraw = os_library_load_proc(lib, Str8_("vkCmdDraw"));

  *(void **)&vkCreateWaylandSurfaceKHR = os_library_load_proc(lib, Str8_("vkCreateWaylandSurfaceKHR"));
  *(void **)&vkGetPhysicalDeviceWaylandPresentationSupportKHR = os_library_load_proc(lib, Str8_("vkGetPhysicalDeviceWaylandPresentationSupportKHR"));

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

  VkResult result = vkCreateInstance(&inst_info, 0, &ramM.instance);
  Assert(result == VK_SUCCESS);

  ////////////////////////////////
  //~ kti: Load Extension Procedures

  *(void **)&vkCreateDebugReportCallbackEXT = vkGetInstanceProcAddr(ramM.instance, "vkCreateDebugReportCallbackEXT");
  *(void **)&vkDestroyDebugReportCallbackEXT = vkGetInstanceProcAddr(ramM.instance, "vkDestroyDebugReportCallbackEXT");
  *(void **)&vkDebugReportMessageEXT = vkGetInstanceProcAddr(ramM.instance, "vkDebugReportMessageEXT");

  ////////////////////////////////
  //~ kti: Register Debug Report Callback

  VkDebugReportCallbackCreateInfoEXT callback_ci = {
    .sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT,
    .flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
    .pfnCallback = &debug_report_callback,
    .pUserData = NULL,
  };

  result = vkCreateDebugReportCallbackEXT(ramM.instance, &callback_ci, NULL, &ramM.vk_debug_callback);
  Assert(result == VK_SUCCESS);

  ////////////////////////////////
  //~ kti: Physical Device

  I1 device_count = 0;
  vkEnumeratePhysicalDevices(ramM.instance, &device_count, 0);
  Assert(device_count > 0);

  VkPhysicalDevice *physical_devices = push_array(arena, VkPhysicalDevice, device_count);
  vkEnumeratePhysicalDevices(ramM.instance, &device_count, physical_devices);

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
        ramM.physical_device = physical_devices[i];
        ramM.physical_devce_properties = props;
        ramM.present_queue_index = j;
        break;
      }
    }

    if (ramM.physical_device) {
      break;
    }
  }

  Assert(ramM.physical_device);

  VkPhysicalDeviceFeatures2 query_device_features2 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
  VkPhysicalDeviceVulkan13Features query_vulkan13_features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
  VkPhysicalDeviceExtendedDynamicStateFeaturesEXT query_extended_dynamic_state_features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT};
  query_device_features2.pNext  = &query_vulkan13_features;
  query_vulkan13_features.pNext = &query_extended_dynamic_state_features;

  vkGetPhysicalDeviceFeatures2(ramM.physical_device, &query_device_features2);

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
    .queueFamilyIndex = ramM.present_queue_index,
    .queueCount = 1,
    .pQueuePriorities = queue_priorities,
  };

  const char *device_extensions[] = { "VK_KHR_swapchain" };
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

  result = vkCreateDevice(ramM.physical_device, &device_info, NULL, &ramM.device);
  Assert(result == VK_SUCCESS);

  vkGetDeviceQueue(ramM.device, ramM.present_queue_index, 0, &ramM.queue);

  ////////////////////////////////
  //~ kti: Pipeline

  VkPipelineLayoutCreateInfo layout_ci = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
  };
  result = vkCreatePipelineLayout(ramM.device, &layout_ci, 0, &ramM.pipeline_layout);
  Assert(result == VK_SUCCESS);

  VkVertexInputBindingDescription binding_description = {
    .binding = 0,
    .stride = sizeof(GFX_Vertex),
    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
  };

  VkVertexInputAttributeDescription attribute_descriptions[] = {
    {.location = 0, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = 0},
    {.location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(GFX_Vertex, r)},
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
    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
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

  if (vert_shader_code.len == 0) {
    printf("Failed to read vertex shader.\n");
  }
  if (frag_shader_code.len == 0) {
    printf("Failed to read fragment shader.\n");
  }

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

  result = vkCreateShaderModule(ramM.device, &vert_shader_ci, 0, &vert_shader);
  Assert(result == VK_SUCCESS);
  result = vkCreateShaderModule(ramM.device, &frag_shader_ci, 0, &frag_shader);
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
    .layout              = ramM.pipeline_layout,
    .renderPass          = VK_NULL_HANDLE,
    .subpass             = 0,
  };

  result = vkCreateGraphicsPipelines(ramM.device, VK_NULL_HANDLE, 1, &pipeline_ci, 0, &ramM.pipeline);

  vkDestroyShaderModule(ramM.device, vert_shader, 0);
  vkDestroyShaderModule(ramM.device, frag_shader, 0);

  ////////////////////////////////
  //~ kti: Vertex Buffer

  GFX_Vertex vertices[3] = {
    {0.0f, -0.5f,  1.0f, 0.0f, 0.0f},
    {0.5f,  0.5f,  0.0f, 1.0f, 0.0f},
    {-0.5f, 0.5f,  0.0f, 0.0f, 1.0f},
  };
  VkBufferCreateInfo vertex_buffer_ci = {
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .size = sizeof(vertices),
    .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
  };
  result = vkCreateBuffer(ramM.device, &vertex_buffer_ci, 0, &ramM.vertex_buffer);
  Assert(result == VK_SUCCESS);

VkMemoryRequirements memory_requirements = {0};
vkGetBufferMemoryRequirements(ramM.device, ramM.vertex_buffer, &memory_requirements);

VkMemoryAllocateInfo alloc_info = {
  .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
  .allocationSize  = memory_requirements.size,
};

VkPhysicalDeviceMemoryProperties mem_properties = {};
vkGetPhysicalDeviceMemoryProperties(ramM.physical_device, &mem_properties);
for EachIndex(i, mem_properties.memoryTypeCount) {
  if (memory_requirements.memoryTypeBits & (1 << i)) {
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    if ((mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
      alloc_info.memoryTypeIndex = i;
      break;
    }
  }
}

result = vkAllocateMemory(ramM.device, &alloc_info, 0, &ramM.vertex_buffer_memory);
Assert(result == VK_SUCCESS);

result = vkBindBufferMemory(ramM.device, ramM.vertex_buffer, ramM.vertex_buffer_memory, 0);
Assert(result == VK_SUCCESS);

void *data;
result = vkMapMemory(ramM.device, ramM.vertex_buffer_memory, 0, sizeof(vertices), 0, &data);
Assert(result == VK_SUCCESS);

memcpy(data, vertices, sizeof(vertices));

vkUnmapMemory(ramM.device, ramM.vertex_buffer_memory);

}

Internal void gfx_vk_recreate_swapchain(Arena *arena, OS_Window *os_window, GFX_Window *vkw) {
  vkDeviceWaitIdle(ramM.device);

  //- kti: Destroy old resources (if they exist).
  if (vkw->swapchain != VK_NULL_HANDLE) {
    vkDestroySwapchainKHR(ramM.device, vkw->swapchain, 0);
    vkw->swapchain = VK_NULL_HANDLE;
  }
  for EachIndex(i, vkw->image_count) {
    vkDestroyImageView(ramM.device, vkw->swapchain_image_views[i], 0);
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
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ramM.physical_device, vkw->surface, &surface_capabilities);

  I1 desired_image_count = 2;
  if (desired_image_count < surface_capabilities.minImageCount) {
    desired_image_count = surface_capabilities.minImageCount;
  } else if (surface_capabilities.maxImageCount != 0 && desired_image_count > surface_capabilities.maxImageCount) {
    desired_image_count = surface_capabilities.maxImageCount;
  }

  VkExtent2D surface_resolution = surface_capabilities.currentExtent;
  if (surface_resolution.width == -1) {
    surface_resolution.width = os_window->width;
    surface_resolution.height = os_window->height;
  }

  VkSurfaceTransformFlagBitsKHR pre_transform = surface_capabilities.currentTransform;
  if(surface_capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
    pre_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  }

  //- kti: FIFO is always supported. We use mailbox if the physical device supports it.
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

  VkResult result = vkCreateSwapchainKHR(ramM.device, &swpachain_ci, NULL, &vkw->swapchain);
  Assert(result == VK_SUCCESS);

  vkw->swapchain_extent = surface_resolution;

  ////////////////////////////////
  //~ kti: Image Views

  I1 prev_image_count = vkw->image_count;
  vkGetSwapchainImagesKHR(ramM.device, vkw->swapchain, &vkw->image_count, 0);
  if (prev_image_count == 0) {
    vkw->swapchain_images = push_array(arena, VkImage, vkw->image_count);
  } else if (prev_image_count != vkw->image_count) {
    Assert(0);
  }
  vkGetSwapchainImagesKHR(ramM.device, vkw->swapchain, &vkw->image_count, vkw->swapchain_images);

  if (prev_image_count == 0) {
    vkw->swapchain_image_views = push_array(arena, VkImageView, vkw->image_count);
  }

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
    vkCreateImageView(ramM.device, &image_view_ci, 0, &vkw->swapchain_image_views[i]);
  }
}

Internal GFX_Window *gfx_window_equip(Arena *arena, OS_Window *window) {
  GFX_Window *vkw = ramM.first_free_vk_window;
  if (vkw == 0) {
    vkw = push_array(arena, GFX_Window, 1);
  } else {
    SLLStackPop(ramM.first_free_vk_window);
  }

  ////////////////////////////////
  //~ kti: Surface

  VkWaylandSurfaceCreateInfoKHR surface_ci = {
    .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
    .display = ramM.display,
    .surface = window->surface,
  };
  VkResult result = vkCreateWaylandSurfaceKHR(ramM.instance, &surface_ci, 0, &vkw->surface);
  Assert(result == VK_SUCCESS);

  VkBool32 supports = vkGetPhysicalDeviceWaylandPresentationSupportKHR(ramM.physical_device, ramM.present_queue_index, ramM.display);
  Assert(supports);

  vkGetPhysicalDeviceSurfaceFormatsKHR(ramM.physical_device, vkw->surface, &vkw->surface_format_count, 0);
  vkw->surface_formats = push_array(arena, VkSurfaceFormatKHR, vkw->surface_format_count);
  vkGetPhysicalDeviceSurfaceFormatsKHR(ramM.physical_device, vkw->surface, &vkw->surface_format_count, vkw->surface_formats);

  vkGetPhysicalDeviceSurfacePresentModesKHR(ramM.physical_device, vkw->surface, &vkw->present_mode_count, 0);
  vkw->present_modes = push_array(arena, VkPresentModeKHR, vkw->present_mode_count);
  vkGetPhysicalDeviceSurfacePresentModesKHR(ramM.physical_device, vkw->surface, &vkw->present_mode_count, vkw->present_modes);

  ////////////////////////////////
  //~ kti: Swapchain

  gfx_vk_recreate_swapchain(arena, window, vkw);

  ////////////////////////////////
  //~ kti: Per Frame Data

  vkw->per_frame = push_array(arena, GFX_Per_Frame, vkw->image_count);

  for EachIndex(i, vkw->image_count) {
    GFX_Per_Frame *frame = &vkw->per_frame[i];

    VkFenceCreateInfo fence_ci = {
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };
    result = vkCreateFence(ramM.device, &fence_ci, 0, &frame->queue_submit_fence);
    Assert(result == VK_SUCCESS);

    VkCommandPoolCreateInfo command_pool_ci = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
      .queueFamilyIndex = ramM.present_queue_index,
    };

    result = vkCreateCommandPool(ramM.device, &command_pool_ci, 0, &frame->command_pool);
    Assert(result == VK_SUCCESS);

    VkCommandBufferAllocateInfo command_buffer_alloc_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = frame->command_pool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1,
    };

    result = vkAllocateCommandBuffers(ramM.device, &command_buffer_alloc_info, &frame->command_buffer);
    Assert(result == VK_SUCCESS);
  }

  return vkw;
}

Internal void gfx_window_unequip(GFX_Window *window) {
  // TODO: Destroy everything.

  vkDestroySwapchainKHR(ramM.device, window->swapchain, 0);
  vkDestroySurfaceKHR(ramM.instance, window->surface, 0);

  SLLStackPush(ramM.first_free_vk_window, window);
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

Internal void gfx_begin_frame(OS_Window *os_window, GFX_Window *vkw) { }

Internal void gfx_end_frame(OS_Window *os_window, GFX_Window *vkw) {
  ////////////////////////////////
  //~ kti: Acquire image

  //- kti: Check for window resize.
  if (os_window->width != vkw->swapchain_extent.width || os_window->height != vkw->swapchain_extent.height) {
    gfx_vk_recreate_swapchain(0, os_window, vkw);
  }

  //- kti: Grab a sempahore.
  VkSemaphore acquire_semaphore;
  if (ramM.recycle_semaphores_count == 0) {
    VkSemaphoreCreateInfo semaphore_ci = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    VkResult result = vkCreateSemaphore(ramM.device, &semaphore_ci, 0, &acquire_semaphore);
    Assert(result == VK_SUCCESS);
  } else {
    acquire_semaphore = ramM.recycle_semaphores[ramM.recycle_semaphores_count - 1];
    ramM.recycle_semaphores_count -= 1;
  }

  //- kti: Grab the next image.
  I1 image_idx = 0;
  VkResult img_acquire_result = VK_RESULT_MAX_ENUM;
  for EachIndex(try, 2) {
    img_acquire_result = vkAcquireNextImageKHR(ramM.device, vkw->swapchain, L1_MAX, acquire_semaphore, VK_NULL_HANDLE, &image_idx);
    if (img_acquire_result == VK_SUBOPTIMAL_KHR || img_acquire_result == VK_ERROR_OUT_OF_DATE_KHR) {
      gfx_vk_recreate_swapchain(0, os_window, vkw);
    } else {
      break;
    }
  }

  if (img_acquire_result != VK_SUCCESS) {
    // - kti: Recycle the semaphore we never used.
    ramM.recycle_semaphores[ramM.recycle_semaphores_count] = acquire_semaphore;
    ramM.recycle_semaphores_count += 1;

    vkQueueWaitIdle(ramM.queue);
    return;
  }

  //- kti: Wait for previous work submitted to this image is complete.
  vkWaitForFences(ramM.device, 1, &vkw->per_frame[image_idx].queue_submit_fence, VK_TRUE, L1_MAX);
  vkResetFences(ramM.device, 1, &vkw->per_frame[image_idx].queue_submit_fence);

  vkResetCommandPool(ramM.device, vkw->per_frame[image_idx].command_pool, 0);

  //- kti: Recycle old semaphore
  VkSemaphore old_semaphore = vkw->per_frame[image_idx].swapchain_acquire_semaphore;
  if (old_semaphore != VK_NULL_HANDLE) {
    ramM.recycle_semaphores[ramM.recycle_semaphores_count] = old_semaphore;
    ramM.recycle_semaphores_count += 1;
  }

  vkw->per_frame[image_idx].swapchain_acquire_semaphore = acquire_semaphore;

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

  VkClearValue clear_value = {.color= {{0.0f, 0.01f, 0.05f, 1.0f}}};

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

  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ramM.pipeline);

  VkViewport viewport = {
    .width = vkw->swapchain_extent.width,
    .height = vkw->swapchain_extent.height,
    .minDepth = 0.0f,
    .maxDepth = 1.0f,
  };

  vkCmdSetViewport(cmd, 0, 1, &viewport);

  VkRect2D scissor = {
    .extent = {
      .width  = vkw->swapchain_extent.width,
      .height = vkw->swapchain_extent.height,
    },
  };
  vkCmdSetScissor(cmd, 0, 1, &scissor);

  vkCmdSetCullMode(cmd, VK_CULL_MODE_NONE);
  vkCmdSetFrontFace(cmd, VK_FRONT_FACE_CLOCKWISE);
  vkCmdSetPrimitiveTopology(cmd, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

  ////////////////////////////////
  //~ kti: Render

  VkDeviceSize offset = {0};
  vkCmdBindVertexBuffers(cmd, 0, 1, &ramM.vertex_buffer, &offset);
  vkCmdDraw(cmd, 3, 1, 0, 0);

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
    result = vkCreateSemaphore(ramM.device, &semaphore_ci, 0, &vkw->per_frame[image_idx].swapchain_release_semaphore);
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
  result = vkQueueSubmit(ramM.queue, 1, &submit_info, vkw->per_frame[image_idx].queue_submit_fence);
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
  result = vkQueuePresentKHR(ramM.queue, &present_info);

  if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR) {
    gfx_vk_recreate_swapchain(0, os_window, vkw);
  } else if (result != VK_SUCCESS) {
    printf("Failed to present swapchain image.\n");
  }
}

// TODO: Do we need to cleanup?

#endif