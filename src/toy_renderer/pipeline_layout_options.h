#pragma once

#include <toy_renderer/gpu_core.h>

namespace ToyRenderer {

struct ResourceBindingLayout {
    uint32_t binding;
    uint32_t count{ 1 };
    ResourceBindingType resourceType;
    ShaderStageFlags shaderStages;

    bool isCompatible(const ResourceBindingLayout &other) const noexcept
    {
        return binding == other.binding &&
                count == other.count &&
                resourceType == other.resourceType;
    }
};

// The following struct describes a bind group (descriptor set) layout and from this we
// will be able to subsequently allocate the actual bind group (descriptor set). Before
// the bind group can be used we will need to populate it with the specified bindings.
struct BindGroupLayout {
    std::vector<ResourceBindingLayout> bindings;
};

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
