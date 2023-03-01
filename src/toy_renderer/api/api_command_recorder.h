#pragma once

#include <toy_renderer/handle.h>
#include <toy_renderer/render_pass_command_recorder_options.h>

namespace ToyRenderer {

struct CommandBuffer_t;
struct RenderPassCommandRecorder_t;
struct MemoryBarrierOptions;
struct BufferCopy;

struct ApiCommandRecorder {
    virtual Handle<RenderPassCommandRecorder_t> beginRenderPass(const RenderPassCommandRecorderOptions &options) = 0;
    virtual void copyBuffer(const BufferCopy &copy) = 0;
    virtual void memoryBarrier(const MemoryBarrierOptions &options) = 0;
    virtual Handle<CommandBuffer_t> finish() = 0;
};

} // namespace ToyRenderer
