#include "vulkan_texture_view.h"

namespace ToyRenderer {

VulkanTextureView::VulkanTextureView(VkImageView _imageView)
    : ApiTextureView()
    , imageView(_imageView)
{
}

} // namespace ToyRenderer
