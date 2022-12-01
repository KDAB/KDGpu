#pragma once

#include <toy_renderer/handle.h>
#include <toy_renderer/gpu_core.h>

#include <vector>

namespace ToyRenderer {

struct PipelineLayout_t;
struct ShaderModule_t;

struct ShaderStage {
    Handle<ShaderModule_t> shaderModule;
    ShaderStageFlagBits stage;
    std::string entryPoint{ "main" };
};

struct PrimitiveOptions {
    PrimitiveTopology topology{ PrimitiveTopology::TriangleList };
    bool primitiveRestart{ false };
    CullModeFlags cullMode{ CullModeFlags(CullModeFlagBits::BackBit) };
    FrontFace frontFace{ FrontFace::CounterClockwise };
    PolygonMode polygonMode{ PolygonMode::Fill };
};

struct GraphicsPipelineOptions {
    std::vector<ShaderStage> shaderStages;
    Handle<PipelineLayout_t> layout;
    // VertexOptions vertex;
    // FragmentOptions fragment;
    // DepthStencilOptions depthStencil;
    // BlendingOptions blending; // Put these onto the FragmentOptions::targets?
    PrimitiveOptions primitive;
    // MultiSampleOptions multisample;
};

} // namespace ToyRenderer
