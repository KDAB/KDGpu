#include "compute_pipeline.h"

namespace ToyRenderer {

ComputePipeline::ComputePipeline() = default;
ComputePipeline::~ComputePipeline() = default;

ComputePipeline::ComputePipeline(GraphicsApi *api,
                                 const Handle<Device_t> &device,
                                 const Handle<ComputePipeline_t> &computePipeline)
    : m_api(api)
    , m_device(device)
    , m_computePipeline(computePipeline)
{
}

bool operator==(const ComputePipeline &a, const ComputePipeline &b)
{
    return a.m_api == b.m_api && a.m_device == b.m_device && a.m_computePipeline == b.m_computePipeline;
}

bool operator!=(const ComputePipeline &a, const ComputePipeline &b)
{
    return !(a == b);
}

} // namespace ToyRenderer
