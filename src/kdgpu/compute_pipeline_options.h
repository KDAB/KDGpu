#pragma once

#include <kdgpu/handle.h>
#include <kdgpu/gpu_core.h>

namespace KDGpu {

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

} // namespace KDGpu
