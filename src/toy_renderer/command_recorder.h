#pragma once

#include <toy_renderer/command_buffer.h>
#include <toy_renderer/handle.h>
#include <toy_renderer/render_pass_command_recorder.h>
#include <toy_renderer/render_pass_command_recorder_options.h>
#include <toy_renderer/toy_renderer_export.h>

namespace ToyRenderer {

class GraphicsApi;

struct CommandRecorder_t;
struct Device_t;
struct Queue_t;

struct CommandRecorderOptions {
    Handle<Queue_t> queue; // The queue on which you wish to submit the recorded commands. If not set, defaults to first queue of the device
};

class TOY_RENDERER_EXPORT CommandRecorder
{
public:
    ~CommandRecorder();

    RenderPassCommandRecorder beginRenderPass(const RenderPassCommandRecorderOptions &options);
    CommandBuffer finish();

protected:
    explicit CommandRecorder(GraphicsApi *api, const Handle<Device_t> &device, const Handle<CommandRecorder_t> &commandRecorder);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<CommandRecorder_t> m_commandRecorder;

    friend class Device;
};

} // namespace ToyRenderer
