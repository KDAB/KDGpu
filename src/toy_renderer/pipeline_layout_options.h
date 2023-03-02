#pragma once

#include <toy_renderer/gpu_core.h>
#include <toy_renderer/bind_group_layout.h>
#include <toy_renderer/handle.h>

namespace ToyRenderer {

struct PushConstantRange {
    uint32_t offset;
    uint32_t size;
    ShaderStageFlags shaderStages;
};

struct BindGroupLayout_t;

struct PipelineLayoutOptions {
    std::vector<Handle<BindGroupLayout_t>> bindGroupLayouts;
    std::vector<PushConstantRange> pushConstantRanges;
};

} // namespace ToyRenderer
