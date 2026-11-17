// Georgy Treshchev 2024.
// ggml-vulkan calls a few Vulkan entry points directly. Instead of linking vulkan-1.lib (Vulkan SDK only), they are forwarded to the
// Vulkan loader that ships with every GPU driver, resolved at runtime. Without a Vulkan driver they return null/no-op and ggml uses the CPU.
#include "RSRThirdPartyPrelude.h"

#if defined(GGML_USE_VULKAN)

#include <vulkan/vulkan_core.h>

#if defined(_WIN32)
#include <windows.h>
static void* RSR_LoadVulkanSymbol(const char* Name)
{
	static HMODULE Loader = LoadLibraryA("vulkan-1.dll");
	return Loader ? reinterpret_cast<void*>(GetProcAddress(Loader, Name)) : nullptr;
}
#else
#include <dlfcn.h>
static void* RSR_LoadVulkanSymbol(const char* Name)
{
	static void* Loader = dlopen("libvulkan.so.1", RTLD_NOW | RTLD_LOCAL);
	return Loader ? dlsym(Loader, Name) : nullptr;
}
#endif

extern "C"
{
VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance Instance, const char* Name)
{
	static PFN_vkGetInstanceProcAddr Fn = reinterpret_cast<PFN_vkGetInstanceProcAddr>(RSR_LoadVulkanSymbol("vkGetInstanceProcAddr"));
	return Fn ? Fn(Instance, Name) : nullptr;
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(VkDevice Device, const char* Name)
{
	static PFN_vkGetDeviceProcAddr Fn = reinterpret_cast<PFN_vkGetDeviceProcAddr>(RSR_LoadVulkanSymbol("vkGetDeviceProcAddr"));
	return Fn ? Fn(Device, Name) : nullptr;
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceFeatures2(VkPhysicalDevice PhysicalDevice, VkPhysicalDeviceFeatures2* Features)
{
	static PFN_vkGetPhysicalDeviceFeatures2 Fn = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures2>(RSR_LoadVulkanSymbol("vkGetPhysicalDeviceFeatures2"));
	if (Fn)
	{
		Fn(PhysicalDevice, Features);
	}
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyBuffer(VkCommandBuffer CommandBuffer, VkBuffer Src, VkBuffer Dst, uint32_t RegionCount, const VkBufferCopy* Regions)
{
	static PFN_vkCmdCopyBuffer Fn = reinterpret_cast<PFN_vkCmdCopyBuffer>(RSR_LoadVulkanSymbol("vkCmdCopyBuffer"));
	if (Fn)
	{
		Fn(CommandBuffer, Src, Dst, RegionCount, Regions);
	}
}
}

#endif
