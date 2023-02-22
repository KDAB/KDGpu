#pragma once

#include <toy_renderer/handle.h>

namespace ToyRenderer {

struct GraphicsPipeline_t;

struct ApiRenderPassCommandRecorder {
    virtual void setPipeline(const Handle<GraphicsPipeline_t> &pipeline) = 0;
    virtual void end() = 0;
};

} // namespace ToyRenderer
