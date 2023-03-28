#include "texture_view.h"

#include <kdgpu/graphics_api.h>
#include <kdgpu/resource_manager.h>
#include <kdgpu/api/api_texture_view.h>

namespace KDGpu {

TextureView::TextureView()
{
}

TextureView::TextureView(GraphicsApi *api, const Handle<TextureView_t> &textureView)
    : m_api(api)
    , m_textureView(textureView)
{
}

TextureView::~TextureView()
{
    if (isValid())
        m_api->resourceManager()->deleteTextureView(handle());
}

TextureView::TextureView(TextureView &&other)
{
    m_api = other.m_api;
    m_textureView = other.m_textureView;

    other.m_api = nullptr;
    other.m_textureView = {};
}

TextureView &TextureView::operator=(TextureView &&other)
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteTextureView(handle());

        m_api = other.m_api;
        m_textureView = other.m_textureView;

        other.m_api = nullptr;
        other.m_textureView = {};
    }
    return *this;
}

bool operator==(const TextureView &a, const TextureView &b)
{
    return a.m_api == b.m_api && a.m_textureView == b.m_textureView;
}

bool operator!=(const TextureView &a, const TextureView &b)
{
    return !(a == b);
}

} // namespace KDGpu
