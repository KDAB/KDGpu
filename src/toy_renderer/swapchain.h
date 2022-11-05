#pragma once

#include <toy_renderer/handle.h>
#include <toy_renderer/toy_renderer_export.h>

namespace ToyRenderer {

class GraphicsApi;

struct Swapchain_t;

class TOY_RENDERER_EXPORT Swapchain
{
public:
    ~Swapchain();

    bool isValid() const noexcept { return m_swapchain.isValid(); }

private:
    explicit Swapchain(GraphicsApi *api, const Handle<Swapchain_t> &swapchain);

    GraphicsApi *m_api{ nullptr };
    Handle<Swapchain_t> m_swapchain;

    friend class Device;
};

} // namespace ToyRenderer
