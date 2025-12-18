/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "raytracing_pass_command_recorder.h"

#include <KDGpu/api/graphics_api_impl.h>

namespace KDGpu {

RayTracingPassCommandRecorder::RayTracingPassCommandRecorder(GraphicsApi *api,
                                                             const Handle<Device_t> &device,
                                                             const Handle<RayTracingPassCommandRecorder_t> &rayTracingPassRecorder)
    : m_api(api)
    , m_device(device)
    , m_rayTracingCommandRecorder(rayTracingPassRecorder)
{
}

RayTracingPassCommandRecorder::~RayTracingPassCommandRecorder()
{
    if (isValid())
        m_api->resourceManager()->deleteRayTracingPassCommandRecorder(handle());
}

RayTracingPassCommandRecorder::RayTracingPassCommandRecorder(RayTracingPassCommandRecorder &&other) noexcept
{
    m_api = std::exchange(other.m_api, nullptr);
    m_device = std::exchange(other.m_device, {});
    m_rayTracingCommandRecorder = std::exchange(other.m_rayTracingCommandRecorder, {});
}

RayTracingPassCommandRecorder &RayTracingPassCommandRecorder::operator=(RayTracingPassCommandRecorder &&other) noexcept
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteRayTracingPassCommandRecorder(handle());

        m_api = std::exchange(other.m_api, nullptr);
        m_device = std::exchange(other.m_device, {});
        m_rayTracingCommandRecorder = std::exchange(other.m_rayTracingCommandRecorder, {});
    }
    return *this;
}

void RayTracingPassCommandRecorder::setPipeline(const Handle<RayTracingPipeline_t> &pipeline)
{
    auto *apiRayTracingPassCommandRecorder = m_api->resourceManager()->getRayTracingPassCommandRecorder(m_rayTracingCommandRecorder);
    apiRayTracingPassCommandRecorder->setPipeline(pipeline);
}

void RayTracingPassCommandRecorder::setBindGroup(uint32_t group, const Handle<BindGroup_t> &bindGroup,
                                                 const Handle<PipelineLayout_t> &pipelineLayout,
                                                 std::span<const uint32_t> dynamicBufferOffsets)
{
    auto *apiRayTracingPassCommandRecorder = m_api->resourceManager()->getRayTracingPassCommandRecorder(m_rayTracingCommandRecorder);
    apiRayTracingPassCommandRecorder->setBindGroup(group, bindGroup, pipelineLayout, dynamicBufferOffsets);
}

void RayTracingPassCommandRecorder::traceRays(const RayTracingCommand &rayTracingCommand)
{
    auto *apiRayTracingPassCommandRecorder = m_api->resourceManager()->getRayTracingPassCommandRecorder(m_rayTracingCommandRecorder);
    apiRayTracingPassCommandRecorder->traceRays(rayTracingCommand);
}

void RayTracingPassCommandRecorder::pushConstant(const PushConstantRange &constantRange, const void *data)
{
    auto *apiRayTracingPassCommandRecorder = m_api->resourceManager()->getRayTracingPassCommandRecorder(m_rayTracingCommandRecorder);
    apiRayTracingPassCommandRecorder->pushConstant(constantRange, data);
}

void RayTracingPassCommandRecorder::pushBindGroup(uint32_t group,
                                                  std::span<const BindGroupEntry> bindGroupEntries,
                                                  const Handle<PipelineLayout_t> &pipelineLayout)
{
    auto *apiRayTracingPassCommandRecorder = m_api->resourceManager()->getRayTracingPassCommandRecorder(m_rayTracingCommandRecorder);
    apiRayTracingPassCommandRecorder->pushBindGroup(group, bindGroupEntries, pipelineLayout);
}

void RayTracingPassCommandRecorder::end()
{
    auto *apiRayTracingPassCommandRecorder = m_api->resourceManager()->getRayTracingPassCommandRecorder(m_rayTracingCommandRecorder);
    apiRayTracingPassCommandRecorder->end();
}

} // namespace KDGpu
