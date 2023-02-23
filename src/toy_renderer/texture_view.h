#pragma once

#include <toy_renderer/handle.h>
#include <toy_renderer/toy_renderer_export.h>

namespace ToyRenderer {

class GraphicsApi;

struct TextureView_t;

class TOY_RENDERER_EXPORT TextureView
{
public:
    TextureView();
    ~TextureView();

    const Handle<TextureView_t> handle() const noexcept { return m_textureView; }
    bool isValid() const noexcept { return m_textureView.isValid(); }

    operator Handle<TextureView_t>() const noexcept { return m_textureView; }

private:
    explicit TextureView(GraphicsApi *api, const Handle<TextureView_t> &textureView);

    GraphicsApi *m_api{ nullptr };
    Handle<TextureView_t> m_textureView;

    friend class Texture;
    friend TOY_RENDERER_EXPORT bool operator==(const TextureView &, const TextureView &);
};

TOY_RENDERER_EXPORT bool operator==(const TextureView &a, const TextureView &b);
TOY_RENDERER_EXPORT bool operator!=(const TextureView &a, const TextureView &b);

} // namespace ToyRenderer
