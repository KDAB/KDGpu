#pragma once

#include <toy_renderer/handle.h>
#include <toy_renderer/gpu_core.h>

namespace ToyRenderer {

struct PipelineLayout_t;
struct ShaderModule_t;

struct ComputePipelineOptions {
    Handle<PipelineLayout_t> layout;
};

} // namespace ToyRenderer
