/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "compute_pass_command_recorder.h"
#include <KDGpu/api/graphics_api_impl.h>

namespace KDGpu {

ComputePassCommandRecorder::ComputePassCommandRecorder(GraphicsApi *api,
                                                       const Handle<Device_t> &device,
                                                       const Handle<ComputePassCommandRecorder_t> &computePassCommandRecorder)
    : m_api(api)
    , m_device(device)
    , m_computePassCommandRecorder(computePassCommandRecorder)
{
}

ComputePassCommandRecorder::~ComputePassCommandRecorder()
{
    if (isValid())
        m_api->resourceManager()->deleteComputePassCommandRecorder(handle());
}

ComputePassCommandRecorder::ComputePassCommandRecorder(ComputePassCommandRecorder &&other) noexcept
{
    m_api = std::exchange(other.m_api, nullptr);
    m_device = std::exchange(other.m_device, {});
    m_computePassCommandRecorder = std::exchange(other.m_computePassCommandRecorder, {});
}

ComputePassCommandRecorder &ComputePassCommandRecorder::operator=(ComputePassCommandRecorder &&other) noexcept
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteComputePassCommandRecorder(handle());

        m_api = std::exchange(other.m_api, nullptr);
        m_device = std::exchange(other.m_device, {});
        m_computePassCommandRecorder = std::exchange(other.m_computePassCommandRecorder, {});
    }
    return *this;
}

void ComputePassCommandRecorder::setPipeline(const Handle<ComputePipeline_t> &pipeline)
{
    auto *apiComputePassCommandRecorder = m_api->resourceManager()->getComputePassCommandRecorder(m_computePassCommandRecorder);
    apiComputePassCommandRecorder->setPipeline(pipeline);
}

void ComputePassCommandRecorder::setBindGroup(uint32_t group, const Handle<BindGroup_t> &bindGroup,
                                              const Handle<PipelineLayout_t> &pipelineLayout,
                                              std::span<const uint32_t> dynamicBufferOffsets)
{
    auto *apiComputePassCommandRecorder = m_api->resourceManager()->getComputePassCommandRecorder(m_computePassCommandRecorder);
    apiComputePassCommandRecorder->setBindGroup(group, bindGroup, pipelineLayout, dynamicBufferOffsets);
}

void ComputePassCommandRecorder::dispatchCompute(const ComputeCommand &command)
{
    auto *apiComputePassCommandRecorder = m_api->resourceManager()->getComputePassCommandRecorder(m_computePassCommandRecorder);
    apiComputePassCommandRecorder->dispatchCompute(command);
}

void ComputePassCommandRecorder::dispatchCompute(std::span<const ComputeCommand> commands)
{
    auto *apiComputePassCommandRecorder = m_api->resourceManager()->getComputePassCommandRecorder(m_computePassCommandRecorder);
    apiComputePassCommandRecorder->dispatchCompute(commands);
}

void ComputePassCommandRecorder::dispatchComputeIndirect(const ComputeCommandIndirect &command)
{
    auto *apiComputePassCommandRecorder = m_api->resourceManager()->getComputePassCommandRecorder(m_computePassCommandRecorder);
    apiComputePassCommandRecorder->dispatchComputeIndirect(command);
}

void ComputePassCommandRecorder::dispatchComputeIndirect(std::span<const ComputeCommandIndirect> commands)
{
    auto *apiComputePassCommandRecorder = m_api->resourceManager()->getComputePassCommandRecorder(m_computePassCommandRecorder);
    apiComputePassCommandRecorder->dispatchComputeIndirect(commands);
}

void ComputePassCommandRecorder::pushConstant(const PushConstantRange &constantRange, const void *data)
{
    auto *apiComputePassCommandRecorder = m_api->resourceManager()->getComputePassCommandRecorder(m_computePassCommandRecorder);
    apiComputePassCommandRecorder->pushConstant(constantRange, data);
}

void ComputePassCommandRecorder::pushBindGroup(uint32_t group,
                                               std::span<const BindGroupEntry> bindGroupEntries,
                                               const Handle<PipelineLayout_t> &pipelineLayout)
{
    auto *apiComputePassCommandRecorder = m_api->resourceManager()->getComputePassCommandRecorder(m_computePassCommandRecorder);
    apiComputePassCommandRecorder->pushBindGroup(group, bindGroupEntries, pipelineLayout);
}

void ComputePassCommandRecorder::end()
{
    auto *apiComputePassCommandRecorder = m_api->resourceManager()->getComputePassCommandRecorder(m_computePassCommandRecorder);
    apiComputePassCommandRecorder->end();
}

} // namespace KDGpu
