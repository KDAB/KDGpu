#pragma once

#include <kdgpu/handle.h>
#include <kdgpu/kdgpu_export.h>

namespace KDGpu {

class GraphicsApi;
struct Device_t;
struct Sampler_t;
struct SamplerOptions;

class KDGPU_EXPORT Sampler
{
public:
    Sampler();
    ~Sampler();

    Sampler(Sampler &&);
    Sampler &operator=(Sampler &&);

    Sampler(const Sampler &) = delete;
    Sampler &operator=(const Sampler &) = delete;

    Handle<Sampler_t> handle() const noexcept { return m_sampler; }
    bool isValid() const noexcept { return m_sampler.isValid(); }

    operator Handle<Sampler_t>() const noexcept { return m_sampler; }

private:
    Sampler(GraphicsApi *api, const Handle<Device_t> &device, const SamplerOptions &options);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<Sampler_t> m_sampler;

    friend class Device;
    friend KDGPU_EXPORT bool operator==(const Sampler &, const Sampler &);
};

KDGPU_EXPORT bool operator==(const Sampler &a, const Sampler &b);
KDGPU_EXPORT bool operator!=(const Sampler &a, const Sampler &b);

} // namespace KDGpu
