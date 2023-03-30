#import <QuartzCore/CAMetalLayer.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_metal.h>

#include <KDGpu/surface_options.h>

#include <spdlog/spdlog.h>

VkSurfaceKHR createVulkanSurface(VkInstance instance, const KDGpu::SurfaceOptions &options)
{
    VkSurfaceKHR vkSurface{ VK_NULL_HANDLE };

    VkMetalSurfaceCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
    createInfo.pLayer = options.layer;
    if (vkCreateMetalSurfaceEXT(instance, &createInfo, nullptr, &vkSurface) != VK_SUCCESS) {
        spdlog::critical("Failed to create Vulkan surface for Metal layer");
        return {};
    }

    return vkSurface;
}