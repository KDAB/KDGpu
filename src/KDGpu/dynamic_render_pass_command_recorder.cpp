/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "dynamic_render_pass_command_recorder.h"

#include <KDGpu/api/graphics_api_impl.h>

namespace KDGpu {

DynamicRenderPassCommandRecorder::DynamicRenderPassCommandRecorder(GraphicsApi *api,
                                                                   const Handle<Device_t> &device,
                                                                   const Handle<DynamicRenderPassCommandRecorder_t> &renderPassCommandRecorder)
    : RenderPassCommandRecorderBase(api, device)
    , m_renderPassCommandRecorder(renderPassCommandRecorder)
{
}

DynamicRenderPassCommandRecorder::~DynamicRenderPassCommandRecorder()
{
    if (isValid())
        m_api->resourceManager()->deleteDynamicRenderPassCommandRecorder(handle());
}

DynamicRenderPassCommandRecorder::DynamicRenderPassCommandRecorder(DynamicRenderPassCommandRecorder &&other) noexcept
{
    m_api = std::exchange(other.m_api, nullptr);
    m_device = std::exchange(other.m_device, {});
    m_renderPassCommandRecorder = std::exchange(other.m_renderPassCommandRecorder, {});
}

DynamicRenderPassCommandRecorder &DynamicRenderPassCommandRecorder::operator=(DynamicRenderPassCommandRecorder &&other) noexcept
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteDynamicRenderPassCommandRecorder(handle());

        m_api = std::exchange(other.m_api, nullptr);
        m_device = std::exchange(other.m_device, {});
        m_renderPassCommandRecorder = std::exchange(other.m_renderPassCommandRecorder, {});
    }
    return *this;
}

VulkanRenderPassCommandRecorderBase *DynamicRenderPassCommandRecorder::backend() const
{
    return m_api->resourceManager()->getDynamicRenderPassCommandRecorder(m_renderPassCommandRecorder);
}

void DynamicRenderPassCommandRecorder::setInputAttachmentMapping(std::span<const std::optional<uint32_t>> colorAttachmentIndices,
                                                                 std::optional<uint32_t> depthAttachmentIndex,
                                                                 std::optional<uint32_t> stencilAttachmentIndex)
{
    auto *apiCommandRecorder = m_api->resourceManager()->getDynamicRenderPassCommandRecorder(m_renderPassCommandRecorder);
    apiCommandRecorder->setInputAttachmentMapping(colorAttachmentIndices, depthAttachmentIndex, stencilAttachmentIndex);
}

void DynamicRenderPassCommandRecorder::setOutputAttachmentMapping(std::span<const std::optional<uint32_t>> remappedOutputs)
{
    auto *apiCommandRecorder = m_api->resourceManager()->getDynamicRenderPassCommandRecorder(m_renderPassCommandRecorder);
    apiCommandRecorder->setOutputAttachmentMapping(remappedOutputs);
}

} // namespace KDGpu
