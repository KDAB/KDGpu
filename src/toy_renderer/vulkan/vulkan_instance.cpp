#include "vulkan_instance.h"

#include <toy_renderer/vulkan/vulkan_adapter.h>
#include <toy_renderer/vulkan/vulkan_resource_manager.h>

#if defined(TOY_RENDERER_PLATFORM_WIN32)
#include <vulkan/vulkan_win32.h>
#endif

namespace ToyRenderer {

VulkanInstance::VulkanInstance(VulkanResourceManager *_vulkanResourceManager, VkInstance _instance)
    : ApiInstance()
    , vulkanResourceManager(_vulkanResourceManager)
    , instance(_instance)
{
}

std::vector<Handle<Adapter_t>> VulkanInstance::queryAdapters()
{
    // Query the physical devices from the instance
    uint32_t adapterCount = 0;
    vkEnumeratePhysicalDevices(instance, &adapterCount, nullptr);
    std::vector<VkPhysicalDevice> physicalDevices(adapterCount);
    vkEnumeratePhysicalDevices(instance, &adapterCount, physicalDevices.data());

    // Store the resulting physical devices in the resource manager so that
    // the Adapters can access them later, and create the Adapters.
    std::vector<Handle<Adapter_t>> adapterHandles;
    adapterHandles.reserve(adapterCount);
    for (uint32_t adapterIndex = 0; adapterIndex < adapterCount; ++adapterIndex) {
        VulkanAdapter vulkanAdapter{ physicalDevices[adapterIndex] };
        adapterHandles.emplace_back(vulkanResourceManager->insertAdapter(vulkanAdapter));
    }

    return adapterHandles;
}

Handle<Surface_t> VulkanInstance::createSurface(const SurfaceOptions &options)
{
    VkSurfaceKHR vkSurface{ VK_NULL_HANDLE };
#if defined(TOY_RENDERER_PLATFORM_WIN32)
    PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR{ nullptr };
    vkCreateWin32SurfaceKHR = reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR"));
    if (!vkCreateWin32SurfaceKHR)
        return {};

    VkWin32SurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hinstance = GetModuleHandle(nullptr);
    createInfo.hwnd = options.hWnd;

    if (vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &vkSurface) != VK_SUCCESS)
        return {};
#endif

#if defined(TOY_RENDERER_PLATFORM_LINUX)
#endif

#if defined(TOY_RENDERER_PLATFORM_MACOS)
#endif

    VulkanSurface vulkanSurface{ vkSurface, instance };
    auto surfaceHandle = vulkanResourceManager->insertSurface(vulkanSurface);
    return surfaceHandle;
}

} // namespace ToyRenderer
