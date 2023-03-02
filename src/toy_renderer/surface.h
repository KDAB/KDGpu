#pragma once

#include <toy_renderer/handle.h>
#include <toy_renderer/toy_renderer_export.h>

namespace ToyRenderer {

struct Surface_t;

class GraphicsApi;

class TOY_RENDERER_EXPORT Surface
{
public:
    Surface();
    ~Surface();

    Surface(Surface &&);
    Surface &operator=(Surface &&);

    Surface(const Surface &) = delete;
    Surface &operator=(const Surface &) = delete;

    Handle<Surface_t> handle() const noexcept { return m_surface; }
    bool isValid() const noexcept { return m_surface.isValid(); }

    operator Handle<Surface_t>() const noexcept { return m_surface; }

private:
    Surface(GraphicsApi *api, const Handle<Surface_t> &surface);

    GraphicsApi *m_api{ nullptr };
    Handle<Surface_t> m_surface;

    friend class Instance;
};

} // namespace ToyRenderer
