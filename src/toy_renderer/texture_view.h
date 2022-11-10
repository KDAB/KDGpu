#pragma once

#include <toy_renderer/handle.h>
#include <toy_renderer/toy_renderer_export.h>

namespace ToyRenderer {

class GraphicsApi;

struct TextureView_t;

class TOY_RENDERER_EXPORT TextureView
{
public:
    ~TextureView();

private:
    explicit TextureView(GraphicsApi *api, const Handle<TextureView_t> &textureView);

    GraphicsApi *m_api{ nullptr };
    Handle<TextureView_t> m_textureView;

    friend class Texture;
};

} // namespace ToyRenderer
