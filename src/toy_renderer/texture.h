#pragma once

#include <toy_renderer/handle.h>
#include <toy_renderer/texture_view.h>
#include <toy_renderer/texture_view_options.h>
#include <toy_renderer/toy_renderer_export.h>

namespace ToyRenderer {

struct Device_t;
struct Texture_t;

class GraphicsApi;

class TOY_RENDERER_EXPORT Texture
{
public:
    Texture();
    ~Texture();

    const Handle<Texture_t> &handle() const noexcept { return m_texture; }
    bool isValid() const noexcept { return m_texture.isValid(); }

    TextureView createView(const TextureViewOptions &options = TextureViewOptions()) const;

private:
    explicit Texture(GraphicsApi *api, const Handle<Device_t> &device, const Handle<Texture_t> &texture);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<Texture_t> m_texture;

    friend class Swapchain;
    friend class Device;
};

} // namespace ToyRenderer
