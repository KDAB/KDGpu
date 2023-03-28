#pragma once

#include <kdgpu/handle.h>
#include <kdgpu/kdgpu_export.h>

namespace KDGpu {

class GraphicsApi;

struct TextureView_t;

class KDGPU_EXPORT TextureView
{
public:
    TextureView();
    ~TextureView();

    TextureView(TextureView &&);
    TextureView &operator=(TextureView &&);

    TextureView(const TextureView &) = delete;
    TextureView &operator=(const TextureView &) = delete;

    const Handle<TextureView_t> handle() const noexcept { return m_textureView; }
    bool isValid() const noexcept { return m_textureView.isValid(); }

    operator Handle<TextureView_t>() const noexcept { return m_textureView; }

private:
    explicit TextureView(GraphicsApi *api, const Handle<TextureView_t> &textureView);

    GraphicsApi *m_api{ nullptr };
    Handle<TextureView_t> m_textureView;

    friend class Texture;
    friend KDGPU_EXPORT bool operator==(const TextureView &, const TextureView &);
};

KDGPU_EXPORT bool operator==(const TextureView &a, const TextureView &b);
KDGPU_EXPORT bool operator!=(const TextureView &a, const TextureView &b);

} // namespace KDGpu
