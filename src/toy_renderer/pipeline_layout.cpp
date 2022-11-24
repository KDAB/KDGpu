#include "pipeline_layout.h"

namespace ToyRenderer {

PipelineLayout::PipelineLayout(GraphicsApi *api,
                               const Handle<Device_t> &device,
                               const Handle<PipelineLayout_t> &pipelineLayout)
    : m_api(api)
    , m_pipelineLayout(pipelineLayout)
    , m_device(device)
{
}

PipelineLayout::~PipelineLayout()
{
}

} // namespace ToyRenderer
