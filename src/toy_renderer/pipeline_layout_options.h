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

struct BindGroupLayout {
    uint32_t setNumber{ 0 };
    std::vector<ResourceBindingLayout> bindings;

    void clear()
    {
        setNumber = 0;
        bindings.clear();
    }
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
