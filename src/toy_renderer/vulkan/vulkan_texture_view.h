#pragma once

#include <toy_renderer/api/api_texture_view.h>

#include <vulkan/vulkan.h>

namespace ToyRenderer {

struct VulkanTextureView : public ApiTextureView {
    explicit VulkanTextureView(VkImageView _imageView);

    VkImageView imageView{ VK_NULL_HANDLE };
};

} // namespace ToyRenderer
