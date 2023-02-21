#include "vulkan_texture_view.h"

namespace ToyRenderer {

VulkanTextureView::VulkanTextureView(VkImageView _imageView, const Handle<Texture_t> &_textureHandle)
    : ApiTextureView()
    , imageView(_imageView)
    , textureHandle(_textureHandle)
{
}

} // namespace ToyRenderer
