#include "command_recorder.h"

#include <toy_renderer/graphics_api.h>
#include <toy_renderer/api/api_command_recorder.h>

namespace ToyRenderer {

CommandRecorder::CommandRecorder(GraphicsApi *api, const Handle<Device_t> &device, const CommandRecorderOptions &options)
    : m_api(api)
    , m_device(device)
    , m_commandRecorder(m_api->resourceManager()->createCommandRecorder(m_device, options))
    , m_level(options.level)
{
    auto apiCommandRecorder = m_api->resourceManager()->getCommandRecorder(m_commandRecorder);
    apiCommandRecorder->begin();
}

CommandRecorder::~CommandRecorder()
{
    if (isValid())
        m_api->resourceManager()->deleteCommandRecorder(handle());
}

CommandRecorder::CommandRecorder(CommandRecorder &&other)
{
    m_api = other.m_api;
    m_device = other.m_device;
    m_commandRecorder = other.m_commandRecorder;
    m_level = other.m_level;

    other.m_api = nullptr;
    other.m_device = {};
    other.m_commandRecorder = {};
    other.m_level = CommandBufferLevel::MaxEnum;
}

CommandRecorder &CommandRecorder::operator=(CommandRecorder &&other)
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteCommandRecorder(handle());

        m_api = other.m_api;
        m_device = other.m_device;
        m_commandRecorder = other.m_commandRecorder;
        m_level = other.m_level;

        other.m_api = nullptr;
        other.m_device = {};
        other.m_commandRecorder = {};
        other.m_level = CommandBufferLevel::MaxEnum;
    }
    return *this;
}

RenderPassCommandRecorder CommandRecorder::beginRenderPass(const RenderPassCommandRecorderOptions &options)
{
    return RenderPassCommandRecorder(m_api, m_device, m_api->resourceManager()->createRenderPassCommandRecorder(m_device, m_commandRecorder, options));
}

ComputePassCommandRecorder CommandRecorder::beginComputePass(const ComputePassCommandRecorderOptions &options)
{
    return ComputePassCommandRecorder(m_api, m_device, m_api->resourceManager()->createComputePassCommandRecorder(m_device, m_commandRecorder, options));
}

void CommandRecorder::copyBuffer(const BufferCopy &copy)
{
    auto apiCommandRecorder = m_api->resourceManager()->getCommandRecorder(m_commandRecorder);
    apiCommandRecorder->copyBuffer(copy);
}

void CommandRecorder::memoryBarrier(const MemoryBarrierOptions &options)
{
    auto apiCommandRecorder = m_api->resourceManager()->getCommandRecorder(m_commandRecorder);
    apiCommandRecorder->memoryBarrier(options);
}

void CommandRecorder::bufferMemoryBarrier(const BufferMemoryBarrierOptions &options)
{
    auto apiCommandRecorder = m_api->resourceManager()->getCommandRecorder(m_commandRecorder);
    apiCommandRecorder->bufferMemoryBarrier(options);
}

void CommandRecorder::textureMemoryBarrier(const TextureMemoryBarrierOptions &options)
{
    auto apiCommandRecorder = m_api->resourceManager()->getCommandRecorder(m_commandRecorder);
    apiCommandRecorder->textureMemoryBarrier(options);
}

CommandBuffer CommandRecorder::finish()
{
    auto apiCommandRecorder = m_api->resourceManager()->getCommandRecorder(m_commandRecorder);
    return CommandBuffer(m_api, m_device, apiCommandRecorder->finish());
}

void CommandRecorder::executeSecondaryCommandBuffer(const Handle<CommandBuffer_t> &secondaryCommandBuffer)
{
    assert(m_level == CommandBufferLevel::Primary);
    auto apiCommandRecorder = m_api->resourceManager()->getCommandRecorder(m_commandRecorder);
    apiCommandRecorder->executeSecondaryCommandBuffer(secondaryCommandBuffer);
}

} // namespace ToyRenderer
