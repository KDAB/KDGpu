#include "graphics_pipeline.h"

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

GraphicsPipeline::~GraphicsPipeline()
{
}

} // namespace ToyRenderer
