#pragma once

#include <KDGpu/api/api_surface.h>

#include <vulkan/vulkan.h>

namespace KDGpu {

struct VulkanSurface : public ApiSurface {
    explicit VulkanSurface(VkSurfaceKHR _surface, VkInstance _instance);

    VkSurfaceKHR surface{ VK_NULL_HANDLE };
    VkInstance instance{ VK_NULL_HANDLE };
};

} // namespace KDGpu
