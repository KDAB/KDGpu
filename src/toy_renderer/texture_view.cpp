#include "texture_view.h"

namespace ToyRenderer {

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
}

bool operator==(const TextureView &a, const TextureView &b)
{
    return a.m_api == b.m_api && a.m_textureView == b.m_textureView;
}

bool operator!=(const TextureView &a, const TextureView &b)
{
    return !(a == b);
}

} // namespace ToyRenderer
