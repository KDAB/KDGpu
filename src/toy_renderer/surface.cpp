#include "surface.h"

namespace ToyRenderer {

Surface::Surface(const Handle<Surface_t> &surface)
    : m_surface(surface)
{
}

Surface::~Surface()
{
}

} // namespace ToyRenderer
