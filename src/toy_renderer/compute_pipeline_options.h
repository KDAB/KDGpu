#pragma once

#include <toy_renderer/handle.h>
#include <toy_renderer/gpu_core.h>

namespace ToyRenderer {

struct PipelineLayout_t;
struct ShaderModule_t;

struct ComputeShaderStage {
    Handle<ShaderModule_t> shaderModule;
    std::string entryPoint{ "main" };
};

struct ComputePipelineOptions {
    Handle<PipelineLayout_t> layout;
    ComputeShaderStage shaderStage;
};

} // namespace ToyRenderer
