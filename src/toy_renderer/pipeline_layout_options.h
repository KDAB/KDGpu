#pragma once

#include <toy_renderer/gpu_core.h>
#include <toy_renderer/bind_group_layout.h>

namespace ToyRenderer {

struct PushConstantRange {
    uint32_t offset;
    uint32_t size;
    ShaderStageFlags shaderStages;
};

struct PipelineLayoutOptions {
    std::vector<BindGroupLayout> bindGroupLayouts;
    std::vector<PushConstantRange> pushConstantRanges;
};

} // namespace ToyRenderer
