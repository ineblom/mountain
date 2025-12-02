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
	VkSwapchainKHR swapchain;
	I1 image_count;
	VkImageView *swapchain_image_views;
	GFX_Per_Frame *per_frame;
};

typedef struct GFX_Vertex GFX_Vertex;
struct GFX_Vertex {
	F1 x, y;
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
Global PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier = 0;
Global PFN_vkWaitForFences vkWaitForFences = 0;
Global PFN_vkResetFences vkResetFences = 0;
Global PFN_vkResetCommandBuffer vkResetCommandBuffer = 0;
Global PFN_vkCreateImageView vkCreateImageView = 0;
Global PFN_vkGetPhysicalDeviceFeatures2 vkGetPhysicalDeviceFeatures2 = 0;
Global PFN_vkGetDeviceQueue vkGetDeviceQueue = 0;
Global PFN_vkCreatePipelineLayout vkCreatePipelineLayout = 0;
Global PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines = 0;
Global PFN_vkDestroyShaderModule vkDestroyShaderModule = 0;
Global PFN_vkCreateShaderModule vkCreateShaderModule = 0;

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
	*(void **)&vkCmdPipelineBarrier = os_library_load_proc(lib, Str8_("vkCmdPipelineBarrier"));
	*(void **)&vkWaitForFences = os_library_load_proc(lib, Str8_("vkWaitForFences"));
	*(void **)&vkResetFences = os_library_load_proc(lib, Str8_("vkResetFences"));
	*(void **)&vkResetCommandBuffer = os_library_load_proc(lib, Str8_("vkResetCommandBuffer"));
	*(void **)&vkCreateImageView = os_library_load_proc(lib, Str8_("vkCreateImageView"));
	*(void **)&vkGetPhysicalDeviceFeatures2 = os_library_load_proc(lib, Str8_("vkGetPhysicalDeviceFeatures2"));
	*(void **)&vkGetDeviceQueue = os_library_load_proc(lib, Str8_("vkGetDeviceQueue"));
	*(void **)&vkCreatePipelineLayout = os_library_load_proc(lib, Str8_("vkCreatePipelineLayout"));
	*(void **)&vkCreateGraphicsPipelines = os_library_load_proc(lib, Str8_("vkCreateGraphicsPipelines"));
	*(void **)&vkDestroyShaderModule = os_library_load_proc(lib, Str8_("vkDestroyShaderModule"));
	*(void **)&vkCreateShaderModule = os_library_load_proc(lib, Str8_("vkCreateShaderModule"));

	*(void **)&vkCreateWaylandSurfaceKHR = os_library_load_proc(lib, Str8_("vkCreateWaylandSurfaceKHR"));
	*(void **)&vkGetPhysicalDeviceWaylandPresentationSupportKHR = os_library_load_proc(lib, Str8_("vkGetPhysicalDeviceWaylandPresentationSupportKHR"));

	////////////////////////////////
	//~ kti: Create Instance

	VkApplicationInfo app_info = {0};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "m";
	app_info.engineVersion = 1;
	app_info.apiVersion = VK_API_VERSION_1_3;

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

	VkInstanceCreateInfo inst_info = {0};
	inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	inst_info.pApplicationInfo = &app_info;
	inst_info.enabledExtensionCount = ArrayCount(required_extensions);
	inst_info.ppEnabledExtensionNames = required_extensions;


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
  Assert(vkCreateDebugReportCallbackEXT);
  *(void **)&vkDestroyDebugReportCallbackEXT = vkGetInstanceProcAddr(ramM.instance, "vkDestroyDebugReportCallbackEXT");
  *(void **)&vkDebugReportMessageEXT = vkGetInstanceProcAddr(ramM.instance, "vkDebugReportMessageEXT");

  ////////////////////////////////
  //~ kti: Register Debug Report Callback

  VkDebugReportCallbackCreateInfoEXT callback_ci = {0};
	callback_ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	callback_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
	callback_ci.pfnCallback = &debug_report_callback;
	callback_ci.pUserData = NULL;

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

	VkDeviceQueueCreateInfo queue_ci = {0};
	queue_ci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_ci.queueFamilyIndex = ramM.present_queue_index;
	queue_ci.queueCount = 1;
	float queue_priorities[] = { 1.0f };
	queue_ci.pQueuePriorities = queue_priorities;

	VkDeviceCreateInfo device_info = {};
	device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_info.pNext = &enable_device_features2;
	device_info.queueCreateInfoCount = 1;
	device_info.pQueueCreateInfos = &queue_ci;
	device_info.enabledLayerCount = ArrayCount(layer_names);
	device_info.ppEnabledLayerNames = layer_names;

	const char *device_extensions[] = { "VK_KHR_swapchain" };
	device_info.enabledExtensionCount = ArrayCount(device_extensions);
	device_info.ppEnabledExtensionNames = device_extensions;

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
    .binding   = 0,
    .stride    = sizeof(GFX_Vertex),
    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
  };

  VkVertexInputAttributeDescription attribute_descriptions[] = {
  	{.location = 0, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = 0},
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

  String8 vert_shader_code = os_read_entire_file(arena, Str8_("./shaders/triangle.vert.spv"));
  String8 frag_shader_code = os_read_entire_file(arena, Str8_("./shaders/triangle.frag.spv"));

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

	VkWaylandSurfaceCreateInfoKHR surface_ci = {0};
	surface_ci.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
	surface_ci.display = ramM.display;
	surface_ci.surface = window->surface;
	VkResult result = vkCreateWaylandSurfaceKHR(ramM.instance, &surface_ci, 0, &vkw->surface);
	Assert(result == VK_SUCCESS);

	VkBool32 supports = vkGetPhysicalDeviceWaylandPresentationSupportKHR(ramM.physical_device, ramM.present_queue_index, ramM.display);
	Assert(supports);

	////////////////////////////////
	//~ kti: Swapchain

	I1 format_count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(ramM.physical_device, vkw->surface, &format_count, 0);
	VkSurfaceFormatKHR *surface_formats = push_array(arena, VkSurfaceFormatKHR, format_count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(ramM.physical_device, vkw->surface, &format_count, surface_formats);

	//- kti: Our pipeline uses BGRA_SRGB so we force it. Not optimal.
	VkFormat color_format = VK_FORMAT_UNDEFINED;

	if (format_count == 1 || surface_formats[0].format == VK_FORMAT_UNDEFINED) {
		color_format = VK_FORMAT_B8G8R8A8_SRGB;
	} else {
		for EachIndex(i, format_count) {
			if (surface_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB) {
				color_format = VK_FORMAT_B8G8R8A8_SRGB;
				break;
			}
		}
	}
	Assert(color_format == VK_FORMAT_B8G8R8A8_SRGB);

	VkColorSpaceKHR color_space = surface_formats[0].colorSpace;

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
		// TODO: Use window size
		surface_resolution.width = window->width;
		surface_resolution.height = window->height;
	}

	VkSurfaceTransformFlagBitsKHR pre_transform = surface_capabilities.currentTransform;
	if(surface_capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
    pre_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}

	I1 present_mode_count = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(ramM.physical_device, vkw->surface, &present_mode_count, 0);
	VkPresentModeKHR *present_modes = push_array(arena, VkPresentModeKHR, present_mode_count);
	vkGetPhysicalDeviceSurfacePresentModesKHR(ramM.physical_device, vkw->surface, &present_mode_count, present_modes);

	//- kti: FIFO is always supported. We use mailbox if the physical device supports it.
	VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
	for EachIndex(i, present_mode_count) {
		if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
			present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
	}

	VkSwapchainCreateInfoKHR swpachain_ci = {0};
	swpachain_ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swpachain_ci.surface = vkw->surface;
	swpachain_ci.minImageCount = desired_image_count;
	swpachain_ci.imageFormat = color_format;
	swpachain_ci.imageColorSpace = color_space;
	swpachain_ci.imageExtent = surface_resolution;
	swpachain_ci.imageArrayLayers = 1;
	swpachain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swpachain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swpachain_ci.preTransform = pre_transform;
	swpachain_ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swpachain_ci.presentMode = present_mode;
	swpachain_ci.clipped = VK_TRUE;

	result = vkCreateSwapchainKHR(ramM.device, &swpachain_ci, NULL, &vkw->swapchain);
	Assert(result == VK_SUCCESS);

	////////////////////////////////
	//~ kti: Image Views

	I1 image_count = 0;
	vkGetSwapchainImagesKHR(ramM.device, vkw->swapchain, &image_count, 0);
	VkImage *present_images = push_array(arena, VkImage, image_count);
	vkGetSwapchainImagesKHR(ramM.device, vkw->swapchain, &image_count, present_images);

	vkw->image_count = image_count;
	vkw->swapchain_image_views = push_array(arena, VkImageView, image_count);

	for EachIndex(i, image_count) {
		VkImageViewCreateInfo image_view_ci = {0};
		image_view_ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_ci.image = present_images[i];
		image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
		image_view_ci.format = color_format;
		image_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		image_view_ci.subresourceRange.baseMipLevel = 0;
		image_view_ci.subresourceRange.levelCount = 1;
		image_view_ci.subresourceRange.baseArrayLayer = 0;
		image_view_ci.subresourceRange.layerCount = 1;
		vkCreateImageView(ramM.device, &image_view_ci, 0, &vkw->swapchain_image_views[i]);
	}

	////////////////////////////////
	//~ kti: Per Frame Data

	vkw->per_frame = push_array(arena, GFX_Per_Frame, image_count);

	for EachIndex(i, image_count) {
		GFX_Per_Frame *frame = &vkw->per_frame[i];

		VkFenceCreateInfo fence_ci = {0};
		fence_ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_ci.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		result = vkCreateFence(ramM.device, &fence_ci, 0, &frame->queue_submit_fence);
		Assert(result == VK_SUCCESS);

		VkCommandPoolCreateInfo command_pool_ci = {0};
		command_pool_ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		command_pool_ci.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		command_pool_ci.queueFamilyIndex = ramM.present_queue_index;

		result = vkCreateCommandPool(ramM.device, &command_pool_ci, 0, &frame->command_pool);
		Assert(result == VK_SUCCESS);

		VkCommandBufferAllocateInfo command_buffer_alloc_info = {0};
		command_buffer_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		command_buffer_alloc_info.commandPool = frame->command_pool;
		command_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		command_buffer_alloc_info.commandBufferCount = 1;

		result = vkAllocateCommandBuffers(ramM.device, &command_buffer_alloc_info, &frame->command_buffer);
		Assert(result == VK_SUCCESS);
	}

	return vkw;
}

Internal void gfx_window_unequip(GFX_Window *window) {
	vkDestroySwapchainKHR(ramM.device, window->swapchain, 0);
	vkDestroySurfaceKHR(ramM.instance, window->surface, 0);

	SLLStackPush(ramM.first_free_vk_window, window);
}

// TODO: Do we need to cleanup?

#endif