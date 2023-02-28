#include "vulkan_texture_view.h"

namespace ToyRenderer {

VulkanTextureView::VulkanTextureView(VkImageView _imageView,
                                     const Handle<Texture_t> &_textureHandle,
                                     const Handle<Device_t> &_deviceHandle)
    : ApiTextureView()
    , imageView(_imageView)
    , textureHandle(_textureHandle)
    , deviceHandle(_deviceHandle)
{
}

} // namespace ToyRenderer
