#pragma once

#include <kdgpu/kdgpu_export.h>

#include <kdgpu/handle.h>

namespace KDGpu {

class GraphicsApi;

struct Device_t;
struct PipelineLayout_t;
struct PipelineLayoutOptions;

class KDGPU_EXPORT PipelineLayout
{
public:
    PipelineLayout();
    ~PipelineLayout();

    PipelineLayout(PipelineLayout &&);
    PipelineLayout &operator=(PipelineLayout &&);

    PipelineLayout(const PipelineLayout &) = delete;
    PipelineLayout &operator=(const PipelineLayout &) = delete;

    const Handle<PipelineLayout_t> &handle() const noexcept { return m_pipelineLayout; }
    bool isValid() const noexcept { return m_pipelineLayout.isValid(); }

    operator Handle<PipelineLayout_t>() const noexcept { return m_pipelineLayout; }

private:
    explicit PipelineLayout(GraphicsApi *api,
                            const Handle<Device_t> &device,
                            const PipelineLayoutOptions &options);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<PipelineLayout_t> m_pipelineLayout;

    friend class Device;
    friend KDGPU_EXPORT bool operator==(const PipelineLayout &, const PipelineLayout &);
};

KDGPU_EXPORT bool operator==(const PipelineLayout &a, const PipelineLayout &b);
KDGPU_EXPORT bool operator!=(const PipelineLayout &a, const PipelineLayout &b);

} // namespace KDGpu
