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

    const Handle<CommandRecorder_t> &handle() const noexcept { return m_commandRecorder; }
    bool isValid() const noexcept { return m_commandRecorder.isValid(); }

    operator Handle<CommandRecorder_t>() const noexcept { return m_commandRecorder; }

    RenderPassCommandRecorder beginRenderPass(const RenderPassCommandRecorderOptions &options);
    void copyBuffer(const Handle<Buffer_t> &src, const Handle<Buffer_t> &dst, size_t byteSize);
    CommandBuffer finish();

protected:
    explicit CommandRecorder(GraphicsApi *api, const Handle<Device_t> &device, const Handle<CommandRecorder_t> &commandRecorder);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<CommandRecorder_t> m_commandRecorder;

    friend class Device;
};

} // namespace ToyRenderer
