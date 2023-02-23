#pragma once

#include <toy_renderer/handle.h>
#include <toy_renderer/toy_renderer_export.h>

namespace ToyRenderer {

struct Surface_t;

class TOY_RENDERER_EXPORT Surface
{
public:
    Surface();
    ~Surface();

    Handle<Surface_t> handle() const noexcept { return m_surface; }
    bool isValid() const noexcept { return m_surface.isValid(); }

    operator Handle<Surface_t>() const noexcept { return m_surface; }

private:
    Surface(const Handle<Surface_t> &surface);

    Handle<Surface_t> m_surface;

    friend class Instance;
};

} // namespace ToyRenderer
