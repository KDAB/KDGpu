#include "compute_pipeline.h"
#include <toy_renderer/graphics_api.h>
#include <toy_renderer/compute_pipeline_options.h>

namespace ToyRenderer {

ComputePipeline::ComputePipeline() = default;
ComputePipeline::~ComputePipeline() = default;

ComputePipeline::ComputePipeline(GraphicsApi *api,
                                 const Handle<Device_t> &device,
                                 const ComputePipelineOptions &options)
    : m_api(api)
    , m_device(device)
    , m_computePipeline(m_api->resourceManager()->createComputePipeline(m_device, options))
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
