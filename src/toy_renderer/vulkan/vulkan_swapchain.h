#pragma once

#include <toy_renderer/api/api_swapchain.h>

#include <toy_renderer/gpu_core.h>
#include <toy_renderer/handle.h>

#include <vulkan/vulkan.h>

namespace ToyRenderer {

class VulkanResourceManager;

struct Device_t;

struct VulkanSwapchain : public ApiSwapchain {
    explicit VulkanSwapchain(VkSwapchainKHR _swapchain,
                             Format _format,
                             TextureUsageFlags _imageUsageFlags,
                             VulkanResourceManager *_vulkanResourceManager,
                             const Handle<Device_t> &_deviceHandle);

    std::vector<Handle<Texture_t>> getTextures() final;

    VkSwapchainKHR swapchain{ VK_NULL_HANDLE };
    Format format;
    TextureUsageFlags imageUsageFlags;
    VulkanResourceManager *vulkanResourceManager{ nullptr };
    Handle<Device_t> deviceHandle;
};

} // namespace ToyRenderer
