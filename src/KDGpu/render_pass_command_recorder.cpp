/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

// NOTE: This file implements the legacy VkRenderPass/VkFramebuffer rendering path.
// For new code, use DynamicRenderPassCommandRecorder for modern Vulkan 1.3 dynamic rendering.

#include "render_pass_command_recorder.h"

#include <KDGpu/api/graphics_api_impl.h>

namespace KDGpu {

RenderPassCommandRecorder::RenderPassCommandRecorder(GraphicsApi *api,
                                                     const Handle<Device_t> &device,
                                                     const Handle<RenderPassCommandRecorder_t> &renderPassCommandRecorder)
    : RenderPassCommandRecorderBase(api, device)
    , m_renderPassCommandRecorder(renderPassCommandRecorder)
{
}

RenderPassCommandRecorder::~RenderPassCommandRecorder()
{
    if (isValid())
        m_api->resourceManager()->deleteRenderPassCommandRecorder(handle());
}

RenderPassCommandRecorder::RenderPassCommandRecorder(RenderPassCommandRecorder &&other) noexcept
{
    m_api = std::exchange(other.m_api, nullptr);
    m_device = std::exchange(other.m_device, {});
    m_renderPassCommandRecorder = std::exchange(other.m_renderPassCommandRecorder, {});
}

RenderPassCommandRecorder &RenderPassCommandRecorder::operator=(RenderPassCommandRecorder &&other) noexcept
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteRenderPassCommandRecorder(handle());

        m_api = std::exchange(other.m_api, nullptr);
        m_device = std::exchange(other.m_device, {});
        m_renderPassCommandRecorder = std::exchange(other.m_renderPassCommandRecorder, {});
    }
    return *this;
}

VulkanRenderPassCommandRecorderBase *RenderPassCommandRecorder::backend() const
{
    return m_api->resourceManager()->getRenderPassCommandRecorder(m_renderPassCommandRecorder);
}

void RenderPassCommandRecorder::nextSubpass()
{
    auto *apiCommandRecorder = m_api->resourceManager()->getRenderPassCommandRecorder(m_renderPassCommandRecorder);
    apiCommandRecorder->nextSubpass();
}

} // namespace KDGpu
