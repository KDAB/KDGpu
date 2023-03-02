#include "pipeline_layout.h"
#include <toy_renderer/graphics_api.h>
#include <toy_renderer/resource_manager.h>
#include <toy_renderer/api/api_pipeline_layout.h>

namespace ToyRenderer {

PipelineLayout::PipelineLayout() = default;

PipelineLayout::PipelineLayout(GraphicsApi *api,
                               const Handle<Device_t> &device,
                               const Handle<PipelineLayout_t> &pipelineLayout)
    : m_api(api)
    , m_pipelineLayout(pipelineLayout)
    , m_device(device)
{
}

PipelineLayout::PipelineLayout(PipelineLayout &&other)
{
    m_api = other.m_api;
    m_device = other.m_device;
    m_pipelineLayout = other.m_pipelineLayout;

    other.m_api = nullptr;
    other.m_device = {};
    other.m_pipelineLayout = {};
}

PipelineLayout &PipelineLayout::operator=(PipelineLayout &&other)
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deletePipelineLayout(handle());

        m_api = other.m_api;
        m_device = other.m_device;
        m_pipelineLayout = other.m_pipelineLayout;

        other.m_api = nullptr;
        other.m_device = {};
        other.m_pipelineLayout = {};
    }
    return *this;
}

PipelineLayout::~PipelineLayout()
{
    if (isValid())
        m_api->resourceManager()->deletePipelineLayout(handle());
}

bool operator==(const PipelineLayout &a, const PipelineLayout &b)
{
    return a.m_api == b.m_api && a.m_device == b.m_device && a.m_pipelineLayout == b.m_pipelineLayout;
}

bool operator!=(const PipelineLayout &a, const PipelineLayout &b)
{
    return !(a == b);
}

} // namespace ToyRenderer
