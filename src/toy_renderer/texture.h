#pragma once

#include <toy_renderer/handle.h>
#include <toy_renderer/toy_renderer_export.h>

namespace ToyRenderer {

struct Texture_t;

class GraphicsApi;

class Texture
{
public:
    ~Texture();

    const Handle<Texture_t> &handle() const noexcept { return m_texture; }

private:
    explicit Texture(GraphicsApi *api, const Handle<Texture_t> &texture);

    GraphicsApi *m_api{ nullptr };
    Handle<Texture_t> m_texture;

    friend class Swapchain;
    friend class Device;
};

} // namespace ToyRenderer
