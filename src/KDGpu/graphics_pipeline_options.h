/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/handle.h>
#include <KDGpu/gpu_core.h>

#include <vector>

// TODO: Can we use std::span in these rather than std::vector?

namespace KDGpu {

struct PipelineLayout_t;
struct ShaderModule_t;

struct ShaderStage {
    Handle<ShaderModule_t> shaderModule;
    ShaderStageFlagBits stage;
    std::string entryPoint{ "main" };
    std::vector<SpecializationConstant> specializationConstants;
};

struct VertexBufferLayout {
    uint32_t binding;
    uint32_t stride;
    VertexRate inputRate{ VertexRate::Vertex };

    friend bool operator==(const VertexBufferLayout &, const VertexBufferLayout &) = default;
};

struct VertexAttribute {
    uint32_t location;
    uint32_t binding;
    Format format;
    DeviceSize offset{ 0 };

    friend bool operator==(const VertexAttribute &, const VertexAttribute &) = default;
};

struct VertexOptions {
    std::vector<VertexBufferLayout> buffers;
    std::vector<VertexAttribute> attributes;

    friend bool operator==(const VertexOptions &, const VertexOptions &) = default;
};

struct StencilOperationOptions {
    StencilOperation failOp{ StencilOperation::Keep };
    StencilOperation passOp{ StencilOperation::Keep };
    StencilOperation depthFailOp{ StencilOperation::Keep };
    CompareOperation compareOp{ CompareOperation::Never };
    uint32_t compareMask{ 0 };
    uint32_t writeMask{ 0 };
    uint32_t reference{ 0 };

    friend bool operator==(const StencilOperationOptions &, const StencilOperationOptions &) = default;
};

struct BlendComponent {
    BlendOperation operation{ BlendOperation::Add };
    BlendFactor srcFactor{ BlendFactor::One };
    BlendFactor dstFactor{ BlendFactor::Zero };

    friend bool operator==(const BlendComponent &, const BlendComponent &) = default;
};

struct BlendOptions {
    bool blendingEnabled{ false };
    BlendComponent color;
    BlendComponent alpha;

    friend bool operator==(const BlendOptions &, const BlendOptions &) = default;
};

struct RenderTargetOptions {
    Format format{ Format::R8G8B8A8_UNORM };
    ColorComponentFlags writeMask{ ColorComponentFlagBits::AllComponents };
    BlendOptions blending;

    friend bool operator==(const RenderTargetOptions &, const RenderTargetOptions &) = default;
};

struct DepthStencilOptions {
    Format format{ Format::UNDEFINED };
    bool depthTestEnabled{ true };
    bool depthWritesEnabled{ false };
    CompareOperation depthCompareOperation{ CompareOperation::Always };
    bool stencilTestEnabled{ false };
    StencilOperationOptions stencilFront;
    StencilOperationOptions stencilBack;
    bool resolveDepthStencil{ false };
    bool depthClampEnabled{ false };

    friend bool operator==(const DepthStencilOptions &, const DepthStencilOptions &) = default;
};

struct DepthBiasOptions {
    bool enabled{ false };
    float biasConstantFactor{ 0.0f };
    float biasClamp = { 0.0f };
    float biasSlopeFactor = { 0.0f };

    friend bool operator==(const DepthBiasOptions &, const DepthBiasOptions &) = default;
};

struct PrimitiveOptions {
    PrimitiveTopology topology{ PrimitiveTopology::TriangleList };
    bool primitiveRestart{ false };
    CullModeFlags cullMode{ CullModeFlagBits::BackBit };
    FrontFace frontFace{ FrontFace::CounterClockwise };
    PolygonMode polygonMode{ PolygonMode::Fill };
    uint32_t patchControlPoints{ 0 };
    DepthBiasOptions depthBias{};
    float lineWidth{ 1.0f };
    bool rasterizerDiscardEnabled{ false };

    friend bool operator==(const PrimitiveOptions &, const PrimitiveOptions &) = default;
};

struct MultisampleOptions {
    SampleCountFlagBits samples{ SampleCountFlagBits::Samples1Bit };
    std::vector<SampleMask> sampleMasks{ 0xFF'FF'FF'FF }; // Must have number of entries equal to number of samples
    bool alphaToCoverageEnabled{ false };

    friend bool operator==(const MultisampleOptions &, const MultisampleOptions &) = default;
};

struct DynamicStateOptions {
    std::vector<DynamicState> enabledDynamicStates;
    friend bool operator==(const DynamicStateOptions &, const DynamicStateOptions &) = default;
};

struct GraphicsPipelineOptions {
    std::string_view label;
    std::vector<ShaderStage> shaderStages;
    Handle<PipelineLayout_t> layout;
    VertexOptions vertex;
    std::vector<RenderTargetOptions> renderTargets;
    DepthStencilOptions depthStencil;
    PrimitiveOptions primitive;
    MultisampleOptions multisample;
    uint32_t viewCount{ 1 };
    DynamicStateOptions dynamicState;
    uint32_t subpassIndex{ 0 };
};

} // namespace KDGpu
