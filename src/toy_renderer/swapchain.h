#pragma once

#include <toy_renderer/handle.h>
#include <toy_renderer/texture.h>
#include <toy_renderer/toy_renderer_export.h>

#include <span>
#include <vector>

namespace ToyRenderer {

class GraphicsApi;

struct Swapchain_t;

class TOY_RENDERER_EXPORT Swapchain
{
public:
    ~Swapchain();

    const Handle<Swapchain_t> &handle() const noexcept { return m_swapchain; }
    bool isValid() const noexcept { return m_swapchain.isValid(); }

    std::span<const Texture> textures() const { return m_textures; }

private:
    explicit Swapchain(GraphicsApi *api, const Handle<Device_t> &device, const Handle<Swapchain_t> &swapchain);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<Swapchain_t> m_swapchain;
    std::vector<Texture> m_textures;

    friend class Device;
};

} // namespace ToyRenderer
