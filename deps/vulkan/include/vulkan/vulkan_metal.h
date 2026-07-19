/* Minimal VK_EXT_metal_surface declarations from the Khronos Vulkan headers. */
#ifndef VULKAN_METAL_H_
#define VULKAN_METAL_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#define VK_EXT_metal_surface 1
#define VK_EXT_METAL_SURFACE_SPEC_VERSION 1
#define VK_EXT_METAL_SURFACE_EXTENSION_NAME "VK_EXT_metal_surface"

typedef VkFlags VkMetalSurfaceCreateFlagsEXT;

typedef struct VkMetalSurfaceCreateInfoEXT {
  VkStructureType sType;
  const void *pNext;
  VkMetalSurfaceCreateFlagsEXT flags;
  const void *pLayer;
} VkMetalSurfaceCreateInfoEXT;

typedef VkResult (VKAPI_PTR *PFN_vkCreateMetalSurfaceEXT)(
  VkInstance instance,
  const VkMetalSurfaceCreateInfoEXT *pCreateInfo,
  const VkAllocationCallbacks *pAllocator,
  VkSurfaceKHR *pSurface);

#ifndef VK_NO_PROTOTYPES
VKAPI_ATTR VkResult VKAPI_CALL vkCreateMetalSurfaceEXT(
  VkInstance instance,
  const VkMetalSurfaceCreateInfoEXT *pCreateInfo,
  const VkAllocationCallbacks *pAllocator,
  VkSurfaceKHR *pSurface);
#endif

#ifdef __cplusplus
}
#endif

#endif
