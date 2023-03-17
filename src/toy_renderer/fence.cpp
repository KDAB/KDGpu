#include "fence.h"

#include <toy_renderer/graphics_api.h>
#include <toy_renderer/resource_manager.h>
#include <toy_renderer/api/api_fence.h>

namespace ToyRenderer {

Fence::Fence() = default;

Fence::Fence(Fence &&other)
{
    m_api = other.m_api;
    m_device = other.m_device;
    m_fence = other.m_fence;

    other.m_api = nullptr;
    other.m_device = {};
    other.m_fence = {};
}

Fence &Fence::operator=(Fence &&other)
{
    if (this != &other) {

        if (isValid())
            m_api->resourceManager()->deleteFence(handle());

        m_api = other.m_api;
        m_device = other.m_device;
        m_fence = other.m_fence;

        other.m_api = nullptr;
        other.m_device = {};
        other.m_fence = {};
    }
    return *this;
}

Fence::Fence(GraphicsApi *api, const Handle<Device_t> &device, const FenceOptions &options)
    : m_api(api)
    , m_device(device)
    , m_fence(m_api->resourceManager()->createFence(m_device, options))
{
}

Fence::~Fence()
{
    if (isValid())
        m_api->resourceManager()->deleteFence(handle());
}

void Fence::reset()
{
    if (isValid())
        m_api->resourceManager()->getFence(handle())->reset();
}

void Fence::wait()
{
    if (isValid())
        m_api->resourceManager()->getFence(handle())->wait();
}

FenceStatus Fence::status() const
{
    return m_api->resourceManager()->getFence(handle())->status();
}

bool operator==(const Fence &a, const Fence &b)
{
    return a.m_api == b.m_api && a.m_device == b.m_device && a.m_fence == b.m_fence;
}

bool operator!=(const Fence &a, const Fence &b)
{
    return !(a == b);
}

} // namespace ToyRenderer
