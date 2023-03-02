#include "sampler.h"

#include <toy_renderer/graphics_api.h>
#include <toy_renderer/resource_manager.h>
#include <toy_renderer/api/api_sampler.h>

namespace ToyRenderer {

Sampler::Sampler() = default;
Sampler::~Sampler()
{
    if (isValid())
        m_api->resourceManager()->deleteSampler(handle());
};

Sampler::Sampler(GraphicsApi *api, const Handle<Device_t> &device, const Handle<Sampler_t> &sampler)
    : m_api(api)
    , m_device(device)
    , m_sampler(sampler)
{
}

Sampler::Sampler(Sampler &&other)
{
    m_api = other.m_api;
    m_device = other.m_device;
    m_sampler = other.m_sampler;

    other.m_api = nullptr;
    other.m_device = {};
    other.m_sampler = {};
}

Sampler &Sampler::operator=(Sampler &&other)
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteSampler(handle());

        m_api = other.m_api;
        m_device = other.m_device;
        m_sampler = other.m_sampler;

        other.m_api = nullptr;
        other.m_device = {};
        other.m_sampler = {};
    }
    return *this;
}

bool operator==(const Sampler &a, const Sampler &b)
{
    return a.m_api == b.m_api && a.m_device == b.m_device && a.m_sampler == b.m_sampler;
}

bool operator!=(const Sampler &a, const Sampler &b)
{
    return !(a == b);
}

} // namespace ToyRenderer
