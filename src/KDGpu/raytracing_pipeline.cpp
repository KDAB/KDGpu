/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "raytracing_pipeline.h"

#include <KDGpu/api/graphics_api_impl.h>
#include <KDGpu/raytracing_pipeline_options.h>

namespace KDGpu {

RayTracingPipeline::RayTracingPipeline() = default;

RayTracingPipeline::~RayTracingPipeline()
{
    if (isValid())
        m_api->resourceManager()->deleteRayTracingPipeline(handle());
}

RayTracingPipeline::RayTracingPipeline(GraphicsApi *api,
                                       const Handle<Device_t> &device,
                                       const RayTracingPipelineOptions &options)
    : m_api(api)
    , m_device(device)
    , m_rayTracingPipeline(m_api->resourceManager()->createRayTracingPipeline(m_device, options))
{
}

RayTracingPipeline::RayTracingPipeline(RayTracingPipeline &&other)
{
    m_api = std::exchange(other.m_api, nullptr);
    m_device = std::exchange(other.m_device, {});
    m_rayTracingPipeline = std::exchange(other.m_rayTracingPipeline, {});
}

RayTracingPipeline &RayTracingPipeline::operator=(RayTracingPipeline &&other)
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteRayTracingPipeline(handle());

        m_api = std::exchange(other.m_api, nullptr);
        m_device = std::exchange(other.m_device, {});
        m_rayTracingPipeline = std::exchange(other.m_rayTracingPipeline, {});
    }
    return *this;
}

std::vector<uint8_t> RayTracingPipeline::shaderGroupHandles(uint32_t firstGroup, uint32_t groupCount) const
{
    auto *apiRtPipeline = m_api->resourceManager()->getRayTracingPipeline(m_rayTracingPipeline);
    return apiRtPipeline->shaderGroupHandles(firstGroup, groupCount);
}

bool operator==(const RayTracingPipeline &a, const RayTracingPipeline &b)
{
    return a.m_api == b.m_api && a.m_device == b.m_device && a.m_rayTracingPipeline == b.m_rayTracingPipeline;
}

bool operator!=(const RayTracingPipeline &a, const RayTracingPipeline &b)
{
    return !(a == b);
}

} // namespace KDGpu
