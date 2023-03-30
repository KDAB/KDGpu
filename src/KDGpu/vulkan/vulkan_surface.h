#pragma once

#include <KDGpu/api/api_surface.h>
#include <KDGpu/kdgpu_export.h>

#include <vulkan/vulkan.h>

namespace KDGpu {

struct KDGPU_EXPORT VulkanSurface : public ApiSurface {
    explicit VulkanSurface(VkSurfaceKHR _surface, VkInstance _instance);

    VkSurfaceKHR surface{ VK_NULL_HANDLE };
    VkInstance instance{ VK_NULL_HANDLE };
};

} // namespace KDGpu
