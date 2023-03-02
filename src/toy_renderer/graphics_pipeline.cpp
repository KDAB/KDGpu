#include "graphics_pipeline.h"
#include <toy_renderer/graphics_api.h>
#include <toy_renderer/resource_manager.h>

namespace ToyRenderer {

GraphicsPipeline::GraphicsPipeline()
{
}

GraphicsPipeline::GraphicsPipeline(GraphicsApi *api,
                                   const Handle<Device_t> &device,
                                   const Handle<GraphicsPipeline_t> &graphicsPipeline)
    : m_api(api)
    , m_device(device)
    , m_graphicsPipeline(graphicsPipeline)
{
}

GraphicsPipeline::GraphicsPipeline(GraphicsPipeline &&other)
{
    m_api = other.m_api;
    m_device = other.m_device;
    m_graphicsPipeline = other.m_graphicsPipeline;

    other.m_api = nullptr;
    other.m_device = {};
    other.m_graphicsPipeline = {};
}

GraphicsPipeline &GraphicsPipeline::operator=(GraphicsPipeline &&other)
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteGraphicsPipeline(handle());

        m_api = other.m_api;
        m_device = other.m_device;
        m_graphicsPipeline = other.m_graphicsPipeline;

        other.m_api = nullptr;
        other.m_device = {};
        other.m_graphicsPipeline = {};
    }
    return *this;
}

GraphicsPipeline::~GraphicsPipeline()
{
    if (isValid())
        m_api->resourceManager()->deleteGraphicsPipeline(handle());
}

bool operator==(const GraphicsPipeline &a, const GraphicsPipeline &b)
{
    return a.m_api == b.m_api && a.m_device == b.m_device && a.m_graphicsPipeline == b.m_graphicsPipeline;
}

bool operator!=(const GraphicsPipeline &a, const GraphicsPipeline &b)
{
    return !(a == b);
}

} // namespace ToyRenderer
