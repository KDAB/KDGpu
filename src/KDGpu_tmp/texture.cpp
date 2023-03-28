#include "texture.h"

#include <kdgpu/graphics_api.h>
#include <kdgpu/resource_manager.h>
#include <kdgpu/api/api_texture.h>
#include <kdgpu/texture_options.h>

namespace KDGpu {

Texture::Texture()
{
}

Texture::Texture(GraphicsApi *api, const Handle<Device_t> &device, const Handle<Texture_t> &texture)
    : m_api(api)
    , m_device(device)
    , m_texture(texture)
{
}

Texture::Texture(GraphicsApi *api, const Handle<Device_t> &device, const TextureOptions &options)
    : Texture(api, device, api->resourceManager()->createTexture(device, options))
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

} // namespace KDGpu
