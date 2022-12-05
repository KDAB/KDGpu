#include "command_recorder.h"

namespace ToyRenderer {

CommandRecorder::CommandRecorder(GraphicsApi *api, const Handle<Device_t> &device)
    : m_api(api)
    , m_device(device)
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

} // namespace ToyRenderer
