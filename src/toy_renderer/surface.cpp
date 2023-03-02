#include "surface.h"

#include <toy_renderer/graphics_api.h>
#include <toy_renderer/resource_manager.h>

namespace ToyRenderer {

Surface::Surface()
{
}

Surface::Surface(GraphicsApi *api, const Handle<Surface_t> &surface)
    : m_api(api)
    , m_surface(surface)
{
}

Surface::Surface(Surface &&other)
{
    m_api = other.m_api;
    m_surface = other.m_surface;

    other.m_api = nullptr;
    other.m_surface = {};
}

Surface &Surface::operator=(Surface &&other)
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteSurface(m_surface);

        m_api = other.m_api;
        m_surface = other.m_surface;

        other.m_api = nullptr;
        other.m_surface = {};
    }
    return *this;
}

Surface::~Surface()
{
    if (isValid())
        m_api->resourceManager()->deleteSurface(m_surface);
}

} // namespace ToyRenderer
