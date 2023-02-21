#pragma once

#include <toy_renderer/api/api_texture_view.h>
#include <toy_renderer/handle.h>

#include <vulkan/vulkan.h>

namespace ToyRenderer {

struct Texture_t;
struct VulkanTextureView : public ApiTextureView {
    explicit VulkanTextureView(VkImageView _imageView, const Handle<Texture_t> &_textureHandle);

    VkImageView imageView{ VK_NULL_HANDLE };
    Handle<Texture_t> textureHandle;
};

} // namespace ToyRenderer
