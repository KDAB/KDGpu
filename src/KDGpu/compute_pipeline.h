#pragma once

#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>

namespace KDGpu {

class GraphicsApi;

struct Device_t;
struct ComputePipeline_t;
struct ComputePipelineOptions;

class KDGPU_EXPORT ComputePipeline
{
public:
    ComputePipeline();
    ~ComputePipeline();

    ComputePipeline(ComputePipeline &&);
    ComputePipeline &operator=(ComputePipeline &&);

    ComputePipeline(const ComputePipeline &) = delete;
    ComputePipeline &operator=(const ComputePipeline &) = delete;

    const Handle<ComputePipeline_t> &handle() const noexcept { return m_computePipeline; }
    bool isValid() const noexcept { return m_computePipeline.isValid(); }

    operator Handle<ComputePipeline_t>() const noexcept { return m_computePipeline; }

private:
    explicit ComputePipeline(GraphicsApi *api,
                             const Handle<Device_t> &device,
                             const ComputePipelineOptions &options);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<ComputePipeline_t> m_computePipeline;

    friend class Device;
    friend KDGPU_EXPORT bool operator==(const ComputePipeline &, const ComputePipeline &);
};

KDGPU_EXPORT bool operator==(const ComputePipeline &a, const ComputePipeline &b);
KDGPU_EXPORT bool operator!=(const ComputePipeline &a, const ComputePipeline &b);

} // namespace KDGpu
