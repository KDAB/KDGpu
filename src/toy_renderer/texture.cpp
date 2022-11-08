#include "texture.h"

namespace ToyRenderer {

Texture::Texture(GraphicsApi *api, const Handle<Texture_t> &texture)
    : m_api(api)
    , m_texture(texture)
{
}

Texture::~Texture()
{
}

} // namespace ToyRenderer
