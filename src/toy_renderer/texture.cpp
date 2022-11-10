#include "texture.h"

#include <toy_renderer/graphics_api.h>
#include <toy_renderer/resource_manager.h>

namespace ToyRenderer {

Texture::Texture(GraphicsApi *api, const Handle<Device_t> &device, const Handle<Texture_t> &texture)
    : m_api(api)
    , m_device(device)
    , m_texture(texture)
{
}

Texture::~Texture()
{
}

TextureView Texture::createView(const TextureViewOptions &options) const
{
    auto textureViewHandle = m_api->resourceManager()->createTextureView(m_device, m_texture, options);
    return TextureView(m_api, textureViewHandle);
}

} // namespace ToyRenderer
