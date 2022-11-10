#include "texture_view.h"

namespace ToyRenderer {

TextureView::TextureView(GraphicsApi *api, const Handle<TextureView_t> &textureView)
    : m_api(api)
    , m_textureView(textureView)
{
}

TextureView::~TextureView()
{
}

} // namespace ToyRenderer
