#include "swapchain.h"

namespace ToyRenderer {

Swapchain::Swapchain(GraphicsApi *api, const Handle<Swapchain_t> &swapchain)
    : m_api(api)
    , m_swapchain(swapchain)
{
}

Swapchain::~Swapchain()
{
}

} // namespace ToyRenderer
