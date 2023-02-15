#include "command_recorder.h"

namespace ToyRenderer {

CommandRecorder::CommandRecorder(GraphicsApi *api, const Handle<Device_t> &device, const Handle<CommandRecorder_t> &commandRecorder)
    : m_api(api)
    , m_device(device)
    , m_commandRecorder(commandRecorder)
{
}

CommandRecorder::~CommandRecorder()
{
}

RenderPassCommandRecorder CommandRecorder::beginRenderPass(const RenderPassOptions &options)
{
    // TODO: Implement me!
    return {};
}

CommandBuffer CommandRecorder::finish()
{
    // TODO: Implement me!
    return {};
}

} // namespace ToyRenderer
