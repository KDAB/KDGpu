/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "command_recorder.h"

#include <KDGpu/api/graphics_api_impl.h>

namespace KDGpu {

CommandRecorder::CommandRecorder(GraphicsApi *api, const Handle<Device_t> &device, const CommandRecorderOptions &options)
    : m_api(api)
    , m_device(device)
    , m_commandRecorder(m_api->resourceManager()->createCommandRecorder(m_device, options))
    , m_level(options.level)
{
    auto *apiCommandRecorder = m_api->resourceManager()->getCommandRecorder(m_commandRecorder);
    apiCommandRecorder->begin();
}

CommandRecorder::~CommandRecorder()
{
    if (isValid())
        m_api->resourceManager()->deleteCommandRecorder(handle());
}

CommandRecorder::CommandRecorder(CommandRecorder &&other) noexcept
{
    m_api = std::exchange(other.m_api, nullptr);
    m_device = std::exchange(other.m_device, {});
    m_commandRecorder = std::exchange(other.m_commandRecorder, {});
    m_level = std::exchange(other.m_level, CommandBufferLevel::MaxEnum);
}

CommandRecorder &CommandRecorder::operator=(CommandRecorder &&other) noexcept
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteCommandRecorder(handle());

        m_api = std::exchange(other.m_api, nullptr);
        m_device = std::exchange(other.m_device, {});
        m_commandRecorder = std::exchange(other.m_commandRecorder, {});
        m_level = std::exchange(other.m_level, CommandBufferLevel::MaxEnum);
    }
    return *this;
}

RenderPassCommandRecorder CommandRecorder::beginRenderPass(const RenderPassCommandRecorderOptions &options) const
{
    return RenderPassCommandRecorder(m_api, m_device, m_api->resourceManager()->createRenderPassCommandRecorder(m_device, m_commandRecorder, options));
}

RenderPassCommandRecorder CommandRecorder::beginRenderPass(const RenderPassCommandRecorderWithRenderPassOptions &options) const
{
    return RenderPassCommandRecorder(m_api, m_device, m_api->resourceManager()->createRenderPassCommandRecorder(m_device, m_commandRecorder, options));
}

RenderPassCommandRecorder CommandRecorder::beginRenderPass(const RenderPassCommandRecorderWithDynamicRenderingOptions &options) const
{
    return RenderPassCommandRecorder(m_api, m_device, m_api->resourceManager()->createRenderPassCommandRecorder(m_device, m_commandRecorder, options));
}

ComputePassCommandRecorder CommandRecorder::beginComputePass(const ComputePassCommandRecorderOptions &options) const
{
    return ComputePassCommandRecorder(m_api, m_device, m_api->resourceManager()->createComputePassCommandRecorder(m_device, m_commandRecorder, options));
}

RayTracingPassCommandRecorder CommandRecorder::beginRayTracingPass(const RayTracingPassCommandRecorderOptions &options) const
{
    return RayTracingPassCommandRecorder(m_api, m_device, m_api->resourceManager()->createRayTracingPassCommandRecorder(m_device, m_commandRecorder, options));
}

TimestampQueryRecorder CommandRecorder::beginTimestampRecording(const TimestampQueryRecorderOptions &options) const
{
    return TimestampQueryRecorder(m_api, m_device, m_api->resourceManager()->createTimestampQueryRecorder(m_device, m_commandRecorder, options));
}

void CommandRecorder::blitTexture(const TextureBlitOptions &options) const
{
    auto *apiCommandRecorder = m_api->resourceManager()->getCommandRecorder(m_commandRecorder);
    apiCommandRecorder->blitTexture(options);
}

void CommandRecorder::clearBuffer(const BufferClear &clear) const
{
    auto *apiCommandRecorder = m_api->resourceManager()->getCommandRecorder(m_commandRecorder);
    apiCommandRecorder->clearBuffer(clear);
}

void CommandRecorder::clearColorTexture(const ClearColorTexture &clear) const
{
    auto *apiCommandRecorder = m_api->resourceManager()->getCommandRecorder(m_commandRecorder);
    apiCommandRecorder->clearColorTexture(clear);
}

void CommandRecorder::clearDepthStencilTexture(const ClearDepthStencilTexture &clear) const
{
    auto *apiCommandRecorder = m_api->resourceManager()->getCommandRecorder(m_commandRecorder);
    apiCommandRecorder->clearDepthStencilTexture(clear);
}

void CommandRecorder::copyBuffer(const BufferCopy &copy) const
{
    auto *apiCommandRecorder = m_api->resourceManager()->getCommandRecorder(m_commandRecorder);
    apiCommandRecorder->copyBuffer(copy);
}

void CommandRecorder::copyBufferToTexture(const BufferToTextureCopy &copy) const
{
    auto *apiCommandRecorder = m_api->resourceManager()->getCommandRecorder(m_commandRecorder);
    apiCommandRecorder->copyBufferToTexture(copy);
}

void CommandRecorder::copyTextureToBuffer(const TextureToBufferCopy &copy) const
{
    auto *apiCommandRecorder = m_api->resourceManager()->getCommandRecorder(m_commandRecorder);
    apiCommandRecorder->copyTextureToBuffer(copy);
}

void CommandRecorder::copyTextureToTexture(const TextureToTextureCopy &copy) const
{
    auto *apiCommandRecorder = m_api->resourceManager()->getCommandRecorder(m_commandRecorder);
    apiCommandRecorder->copyTextureToTexture(copy);
}

void CommandRecorder::updateBuffer(const BufferUpdate &update) const
{
    auto *apiCommandRecorder = m_api->resourceManager()->getCommandRecorder(m_commandRecorder);
    apiCommandRecorder->updateBuffer(update);
}

void CommandRecorder::memoryBarrier(const MemoryBarrierOptions &options) const
{
    auto *apiCommandRecorder = m_api->resourceManager()->getCommandRecorder(m_commandRecorder);
    apiCommandRecorder->memoryBarrier(options);
}

void CommandRecorder::bufferMemoryBarrier(const BufferMemoryBarrierOptions &options) const
{
    auto *apiCommandRecorder = m_api->resourceManager()->getCommandRecorder(m_commandRecorder);
    apiCommandRecorder->bufferMemoryBarrier(options);
}

void CommandRecorder::textureMemoryBarrier(const TextureMemoryBarrierOptions &options) const
{
    auto *apiCommandRecorder = m_api->resourceManager()->getCommandRecorder(m_commandRecorder);
    apiCommandRecorder->textureMemoryBarrier(options);
}

CommandBuffer CommandRecorder::finish() const
{
    auto *apiCommandRecorder = m_api->resourceManager()->getCommandRecorder(m_commandRecorder);
    return CommandBuffer(m_api, m_device, apiCommandRecorder->finish());
}

void CommandRecorder::executeSecondaryCommandBuffer(const Handle<CommandBuffer_t> &secondaryCommandBuffer) const
{
    assert(m_level == CommandBufferLevel::Primary);
    auto *apiCommandRecorder = m_api->resourceManager()->getCommandRecorder(m_commandRecorder);
    apiCommandRecorder->executeSecondaryCommandBuffer(secondaryCommandBuffer);
}

void CommandRecorder::resolveTexture(const TextureResolveOptions &options) const
{
    auto *apiCommandRecorder = m_api->resourceManager()->getCommandRecorder(m_commandRecorder);
    apiCommandRecorder->resolveTexture(options);
}

void CommandRecorder::buildAccelerationStructures(const BuildAccelerationStructureOptions &options) const
{
    auto *apiCommandRecorder = m_api->resourceManager()->getCommandRecorder(m_commandRecorder);
    apiCommandRecorder->buildAccelerationStructures(options);
}

void CommandRecorder::beginDebugLabel(const DebugLabelOptions &options) const
{
    auto *apiCommandRecorder = m_api->resourceManager()->getCommandRecorder(m_commandRecorder);
    apiCommandRecorder->beginDebugLabel(options);
}

void CommandRecorder::endDebugLabel() const
{
    auto *apiCommandRecorder = m_api->resourceManager()->getCommandRecorder(m_commandRecorder);
    apiCommandRecorder->endDebugLabel();
}

} // namespace KDGpu
