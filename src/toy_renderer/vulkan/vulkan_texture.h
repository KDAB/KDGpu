#pragma once

#include <toy_renderer/api/api_texture.h>

#include <vulkan/vulkan.h>

namespace ToyRenderer {

class VulkanResourceManager;

struct VulkanTexture : public ApiTexture {
    VulkanTexture(VkImage _image, VkDevice _device, VulkanResourceManager *_vulkanResourceManager);

    // TODO: Implement creation of texture views (subset of texture)
    // Handle<TextureView_t> createView() final;

    VulkanResourceManager *vulkanResourceManager{ nullptr };
    VkImage image{ VK_NULL_HANDLE };
    VkDevice device{ VK_NULL_HANDLE };
};

} // namespace ToyRenderer
