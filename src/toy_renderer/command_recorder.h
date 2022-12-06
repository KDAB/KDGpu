#pragma once

#include <toy_renderer/command_buffer.h>
#include <toy_renderer/handle.h>
#include <toy_renderer/render_pass_command_recorder.h>
#include <toy_renderer/render_pass_options.h>
#include <toy_renderer/toy_renderer_export.h>

namespace ToyRenderer {

class GraphicsApi;

struct Device_t;

class TOY_RENDERER_EXPORT CommandRecorder
{
public:
    ~CommandRecorder();

    RenderPassCommandRecorder beginRenderPass(const RenderPassOptions &options);
    CommandBuffer finish();

protected:
    explicit CommandRecorder(GraphicsApi *api, const Handle<Device_t> &device);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;

    friend class Device;
};

} // namespace ToyRenderer
