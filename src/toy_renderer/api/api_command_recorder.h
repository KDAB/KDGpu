#pragma once

#include <toy_renderer/handle.h>
#include <toy_renderer/render_pass_options.h>

namespace ToyRenderer {

struct CommandBuffer_t;
struct RenderPassCommandRecorder_t;

struct ApiCommandRecorder {
    virtual Handle<RenderPassCommandRecorder_t> beginRenderPass(const RenderPassOptions &options) = 0;
    virtual Handle<CommandBuffer_t> finish() = 0;
};

} // namespace ToyRenderer
