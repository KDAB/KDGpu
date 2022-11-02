#pragma once

#include <toy_renderer/api/api_surface.h>

#include <vulkan/vulkan.h>

namespace ToyRenderer {

struct VulkanSurface : public ApiSurface {
    explicit VulkanSurface(VkSurfaceKHR _surface, VkInstance _instance);

    VkSurfaceKHR surface{ VK_NULL_HANDLE };
    VkInstance instance{ VK_NULL_HANDLE };
};

} // namespace ToyRenderer
