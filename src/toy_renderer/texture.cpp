#include "texture.h"

#include <toy_renderer/graphics_api.h>
#include <toy_renderer/resource_manager.h>
#include <toy_renderer/api/api_texture.h>

namespace ToyRenderer {

Texture::Texture()
{
}

Texture::Texture(GraphicsApi *api, const Handle<Device_t> &device, const Handle<Texture_t> &texture)
    : m_api(api)
    , m_device(device)
    , m_texture(texture)
{
}

Texture::Texture(Texture &&other)
{
    m_api = other.m_api;
    m_device = other.m_device;
    m_texture = other.m_texture;

    other.m_api = nullptr;
    other.m_device = {};
    other.m_texture = {};
}

Texture &Texture::operator=(Texture &&other)
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteTexture(handle());

        m_api = other.m_api;
        m_device = other.m_device;
        m_texture = other.m_texture;

        other.m_api = nullptr;
        other.m_device = {};
        other.m_texture = {};
    }
    return *this;
}

Texture::~Texture()
{
    if (isValid())
        m_api->resourceManager()->deleteTexture(handle());
}

TextureView Texture::createView(const TextureViewOptions &options) const
{
    auto textureViewHandle = m_api->resourceManager()->createTextureView(m_device, m_texture, options);
    return TextureView(m_api, textureViewHandle);
}

bool operator==(const Texture &a, const Texture &b)
{
    return a.m_api == b.m_api && a.m_device == b.m_device && a.m_texture == b.m_texture;
}

bool operator!=(const Texture &a, const Texture &b)
{
    return !(a == b);
}

} // namespace ToyRenderer
