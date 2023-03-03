#pragma once

#include <toy_renderer/command_buffer.h>
#include <toy_renderer/handle.h>
#include <toy_renderer/compute_pass_command_recorder.h>
#include <toy_renderer/render_pass_command_recorder.h>
#include <toy_renderer/render_pass_command_recorder_options.h>
#include <toy_renderer/toy_renderer_export.h>
#include <toy_renderer/memory_barrier.h>

namespace ToyRenderer {

class GraphicsApi;

struct CommandRecorder_t;
struct Device_t;
struct Queue_t;

struct CommandRecorderOptions {
    Handle<Queue_t> queue; // The queue on which you wish to submit the recorded commands. If not set, defaults to first queue of the device
    CommandBufferLevel level{ CommandBufferLevel::Primary };
};

struct BufferCopy {
    Handle<Buffer_t> src;
    size_t srcOffset{ 0 };
    Handle<Buffer_t> dst;
    size_t dstOffset{ 0 };
    size_t byteSize{ 0 };
};

struct MemoryBarrierOptions {
    PipelineStageFlags srcStages;
    PipelineStageFlags dstStages;
    std::vector<MemoryBarrier> memoryBarriers;
};

class TOY_RENDERER_EXPORT CommandRecorder
{
public:
    ~CommandRecorder();

    CommandRecorder(CommandRecorder &&);
    CommandRecorder &operator=(CommandRecorder &&);

    CommandRecorder(const CommandRecorder &) = delete;
    CommandRecorder &operator=(const CommandRecorder &) = delete;

    const Handle<CommandRecorder_t> &handle() const noexcept { return m_commandRecorder; }
    bool isValid() const noexcept { return m_commandRecorder.isValid(); }

    operator Handle<CommandRecorder_t>() const noexcept { return m_commandRecorder; }

    RenderPassCommandRecorder beginRenderPass(const RenderPassCommandRecorderOptions &options);
    ComputePassCommandRecorder beginComputePass(const ComputePassCommandRecorderOptions &options = {});
    void copyBuffer(const BufferCopy &copy);
    void memoryBarrier(const MemoryBarrierOptions &options);
    void executeSecondaryCommandBuffer(const Handle<CommandBuffer_t> &secondaryCommandBuffer);

    CommandBuffer finish();

protected:
    explicit CommandRecorder(GraphicsApi *api, const Handle<Device_t> &device, const CommandRecorderOptions &options);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<CommandRecorder_t> m_commandRecorder;
    CommandBufferLevel m_level;

    friend class Device;
};

} // namespace ToyRenderer
