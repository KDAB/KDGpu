#pragma once

#include <kdgpu/handle.h>
#include <kdgpu/texture_view.h>
#include <kdgpu/texture_view_options.h>
#include <kdgpu/kdgpu_export.h>

namespace KDGpu {

struct Device_t;
struct Texture_t;

class GraphicsApi;
struct TextureOptions;

class KDGPU_EXPORT Texture
{
public:
    Texture();
    ~Texture();

    Texture(Texture &&);
    Texture &operator=(Texture &&);

    Texture(const Texture &) = delete;
    Texture &operator=(const Texture &) = delete;

    const Handle<Texture_t> &handle() const noexcept { return m_texture; }
    bool isValid() const noexcept { return m_texture.isValid(); }

    operator Handle<Texture_t>() const noexcept { return m_texture; }

    TextureView createView(const TextureViewOptions &options = TextureViewOptions()) const;

private:
    explicit Texture(GraphicsApi *api, const Handle<Device_t> &device, const TextureOptions &options);
    explicit Texture(GraphicsApi *api, const Handle<Device_t> &device, const Handle<Texture_t> &handle); // From Swapchain

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<Texture_t> m_texture;

    friend class Swapchain;
    friend class Device;
    friend KDGPU_EXPORT bool operator==(const Texture &, const Texture &);
};

KDGPU_EXPORT bool operator==(const Texture &a, const Texture &b);
KDGPU_EXPORT bool operator!=(const Texture &a, const Texture &b);

} // namespace KDGpu
