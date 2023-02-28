#include "sampler.h"

namespace ToyRenderer {

Sampler::Sampler() = default;
Sampler::~Sampler() = default;

Sampler::Sampler(GraphicsApi *api, const Handle<Device_t> &device, const Handle<Sampler_t> &sampler)
    : m_api(api)
    , m_device(device)
    , m_sampler(sampler)
{
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
