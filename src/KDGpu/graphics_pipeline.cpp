/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "graphics_pipeline.h"
#include <KDGpu/graphics_api.h>
#include <KDGpu/api/graphics_api_impl.h>

namespace KDGpu {

GraphicsPipeline::GraphicsPipeline()
{
}

GraphicsPipeline::GraphicsPipeline(GraphicsApi *api,
                                   const Handle<Device_t> &device,
                                   const GraphicsPipelineOptions &options)
    : m_api(api)
    , m_device(device)
    , m_graphicsPipeline(m_api->resourceManager()->createGraphicsPipeline(m_device, options))
{
}

GraphicsPipeline::GraphicsPipeline(GraphicsPipeline &&other)
{
    m_api = std::exchange(other.m_api, nullptr);
    m_device = std::exchange(other.m_device, {});
    m_graphicsPipeline = std::exchange(other.m_graphicsPipeline, {});
}

GraphicsPipeline &GraphicsPipeline::operator=(GraphicsPipeline &&other)
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteGraphicsPipeline(handle());

        m_api = std::exchange(other.m_api, nullptr);
        m_device = std::exchange(other.m_device, {});
        m_graphicsPipeline = std::exchange(other.m_graphicsPipeline, {});
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

} // namespace KDGpu
