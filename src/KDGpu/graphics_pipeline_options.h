/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/handle.h>
#include <KDGpu/gpu_core.h>
#include <KDGpu/handle.h>
#include <KDFoundation/hashutils.h>

#include <vector>
#include <optional>

namespace KDGpu {

struct PipelineLayout_t;
struct ShaderModule_t;
struct RenderPass_t;

struct ShaderStage {
    Handle<ShaderModule_t> shaderModule;
    ShaderStageFlagBits stage;
    std::string entryPoint{ "main" };
    std::vector<SpecializationConstant> specializationConstants;

    friend bool operator==(const ShaderStage &, const ShaderStage &) = default;
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

struct DynamicAttachmentMapping {
    bool enabled;
    uint32_t remappedIndex;

    friend bool operator==(const DynamicAttachmentMapping &, const DynamicAttachmentMapping &) = default;
};

struct DynamicInputAttachmentLocations {
    std::vector<DynamicAttachmentMapping> inputColorAttachments;
    DynamicAttachmentMapping inputDepthAttachment;
    DynamicAttachmentMapping inputStencilAttachment;

    friend bool operator==(const DynamicInputAttachmentLocations &, const DynamicInputAttachmentLocations &) = default;
};

struct DynamicOutputAttachmentLocations {
    std::vector<DynamicAttachmentMapping> outputAttachments;

    friend bool operator==(const DynamicOutputAttachmentLocations &, const DynamicOutputAttachmentLocations &) = default;
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

    // Explicit RenderPass Usage (incompatible with dynamic rendering)
    // Use implicit RenderPass if none specified
    Handle<RenderPass_t> renderPass;
    uint32_t subpassIndex{ 0 };

    // Only available if DynamicRendering feature is available and enabled
    struct DynamicRendering {
        bool enabled = false;
        std::optional<DynamicInputAttachmentLocations> dynamicInputLocations;
        std::optional<DynamicOutputAttachmentLocations> dynamicOutputLocations;

        friend bool operator==(const DynamicRendering &, const DynamicRendering &) = default;
    };
    DynamicRendering dynamicRendering{};

    friend bool operator==(const GraphicsPipelineOptions &, const GraphicsPipelineOptions &) = default;
};

} // namespace KDGpu

namespace std {

template<>
struct hash<KDGpu::ShaderStage> {
    size_t operator()(const KDGpu::ShaderStage &stage) const noexcept
    {
        uint64_t hash = 0;
        KDFoundation::hash_combine(hash, std::hash<std::string>()(stage.entryPoint));
        KDFoundation::hash_combine(hash, std::hash<KDGpu::Handle<KDGpu::ShaderModule_t>>()(stage.shaderModule));
        KDFoundation::hash_combine(hash, static_cast<uint32_t>(stage.stage));
        for (const auto &specConst : stage.specializationConstants) {
            KDFoundation::hash_combine(hash, specConst.constantId);
            std::visit([&hash](const auto &value) { KDFoundation::hash_combine(hash, value); }, specConst.value);
        }
        return hash;
    }
};

template<>
struct hash<KDGpu::VertexBufferLayout> {
    size_t operator()(const KDGpu::VertexBufferLayout &layout) const noexcept
    {
        uint64_t hash = 0;
        KDFoundation::hash_combine(hash, layout.binding);
        KDFoundation::hash_combine(hash, layout.stride);
        KDFoundation::hash_combine(hash, static_cast<uint32_t>(layout.inputRate));
        return hash;
    }
};

template<>
struct hash<KDGpu::VertexAttribute> {
    size_t operator()(const KDGpu::VertexAttribute &attribute) const noexcept
    {
        uint64_t hash = 0;
        KDFoundation::hash_combine(hash, attribute.location);
        KDFoundation::hash_combine(hash, attribute.binding);
        KDFoundation::hash_combine(hash, static_cast<uint32_t>(attribute.format));
        KDFoundation::hash_combine(hash, attribute.offset);
        return hash;
    }
};

template<>
struct hash<KDGpu::VertexOptions> {
    size_t operator()(const KDGpu::VertexOptions &options) const noexcept
    {
        uint64_t hash = 0;
        for (const auto &buffer : options.buffers) {
            KDFoundation::hash_combine(hash, buffer);
        }
        for (const auto &attribute : options.attributes) {
            KDFoundation::hash_combine(hash, attribute);
        }
        return hash;
    }
};

template<>
struct hash<KDGpu::BlendComponent> {
    size_t operator()(const KDGpu::BlendComponent &component) const noexcept
    {
        uint64_t hash = 0;
        KDFoundation::hash_combine(hash, static_cast<uint32_t>(component.operation));
        KDFoundation::hash_combine(hash, static_cast<uint32_t>(component.srcFactor));
        KDFoundation::hash_combine(hash, static_cast<uint32_t>(component.dstFactor));
        return hash;
    }
};

template<>
struct hash<KDGpu::BlendOptions> {
    size_t operator()(const KDGpu::BlendOptions &options) const noexcept
    {
        uint64_t hash = 0;
        KDFoundation::hash_combine(hash, options.blendingEnabled);
        KDFoundation::hash_combine(hash, options.color);
        KDFoundation::hash_combine(hash, options.alpha);
        return hash;
    }
};

template<>
struct hash<KDGpu::RenderTargetOptions> {
    size_t operator()(const KDGpu::RenderTargetOptions &options) const noexcept
    {
        uint64_t hash = 0;
        KDFoundation::hash_combine(hash, static_cast<uint32_t>(options.format));
        KDFoundation::hash_combine(hash, static_cast<uint32_t>(options.writeMask.toInt()));
        KDFoundation::hash_combine(hash, options.blending.blendingEnabled);
        KDFoundation::hash_combine(hash, options.blending.color);
        KDFoundation::hash_combine(hash, options.blending.alpha);
        return hash;
    }
};

template<>
struct hash<KDGpu::StencilOperationOptions> {
    size_t operator()(const KDGpu::StencilOperationOptions &options) const noexcept
    {
        uint64_t hash = 0;
        KDFoundation::hash_combine(hash, static_cast<uint32_t>(options.failOp));
        KDFoundation::hash_combine(hash, static_cast<uint32_t>(options.passOp));
        KDFoundation::hash_combine(hash, static_cast<uint32_t>(options.depthFailOp));
        KDFoundation::hash_combine(hash, static_cast<uint32_t>(options.compareOp));
        KDFoundation::hash_combine(hash, options.compareMask);
        KDFoundation::hash_combine(hash, options.writeMask);
        KDFoundation::hash_combine(hash, options.reference);
        return hash;
    }
};

template<>
struct hash<KDGpu::DepthStencilOptions> {
    size_t operator()(const KDGpu::DepthStencilOptions &options) const noexcept
    {
        uint64_t hash = 0;
        KDFoundation::hash_combine(hash, static_cast<uint32_t>(options.format));
        KDFoundation::hash_combine(hash, options.depthTestEnabled);
        KDFoundation::hash_combine(hash, options.depthWritesEnabled);
        KDFoundation::hash_combine(hash, static_cast<uint32_t>(options.depthCompareOperation));
        KDFoundation::hash_combine(hash, options.stencilTestEnabled);
        KDFoundation::hash_combine(hash, options.stencilFront);
        KDFoundation::hash_combine(hash, options.stencilBack);
        KDFoundation::hash_combine(hash, options.resolveDepthStencil);
        KDFoundation::hash_combine(hash, options.depthClampEnabled);
        return hash;
    }
};

template<>
struct hash<KDGpu::DepthBiasOptions> {
    size_t operator()(const KDGpu::DepthBiasOptions &options) const noexcept
    {
        uint64_t hash = 0;
        KDFoundation::hash_combine(hash, options.enabled);
        KDFoundation::hash_combine(hash, options.biasConstantFactor);
        KDFoundation::hash_combine(hash, options.biasClamp);
        KDFoundation::hash_combine(hash, options.biasSlopeFactor);
        return hash;
    }
};

template<>
struct hash<KDGpu::PrimitiveOptions> {
    size_t operator()(const KDGpu::PrimitiveOptions &options) const noexcept
    {
        uint64_t hash = 0;
        KDFoundation::hash_combine(hash, static_cast<uint32_t>(options.topology));
        KDFoundation::hash_combine(hash, options.primitiveRestart);
        KDFoundation::hash_combine(hash, static_cast<uint32_t>(options.cullMode.toInt()));
        KDFoundation::hash_combine(hash, static_cast<uint32_t>(options.frontFace));
        KDFoundation::hash_combine(hash, static_cast<uint32_t>(options.polygonMode));
        KDFoundation::hash_combine(hash, options.patchControlPoints);
        KDFoundation::hash_combine(hash, options.depthBias);
        KDFoundation::hash_combine(hash, options.lineWidth);
        KDFoundation::hash_combine(hash, options.rasterizerDiscardEnabled);
        return hash;
    }
};

template<>
struct hash<KDGpu::MultisampleOptions> {
    size_t operator()(const KDGpu::MultisampleOptions &options) const noexcept
    {
        uint64_t hash = 0;
        KDFoundation::hash_combine(hash, static_cast<uint32_t>(options.samples));
        for (const auto &mask : options.sampleMasks) {
            KDFoundation::hash_combine(hash, mask);
        }
        KDFoundation::hash_combine(hash, options.alphaToCoverageEnabled);
        return hash;
    }
};

template<>
struct hash<KDGpu::DynamicStateOptions> {
    size_t operator()(const KDGpu::DynamicStateOptions &options) const noexcept
    {
        uint64_t hash = 0;
        for (const auto &state : options.enabledDynamicStates) {
            KDFoundation::hash_combine(hash, static_cast<uint32_t>(state));
        }
        return hash;
    }
};

template<>
struct hash<KDGpu::GraphicsPipelineOptions> {
    size_t operator()(const KDGpu::GraphicsPipelineOptions &options) const noexcept
    {
        uint64_t hash = 0;
        KDFoundation::hash_combine(hash, std::hash<std::string_view>()(options.label));
        for (const auto &shaderStage : options.shaderStages) {
            KDFoundation::hash_combine(hash, shaderStage);
        }
        KDFoundation::hash_combine(hash, options.layout);
        KDFoundation::hash_combine(hash, options.vertex);
        for (const auto &renderTarget : options.renderTargets) {
            KDFoundation::hash_combine(hash, renderTarget);
        }
        KDFoundation::hash_combine(hash, options.depthStencil);
        KDFoundation::hash_combine(hash, options.primitive);
        KDFoundation::hash_combine(hash, options.multisample);
        KDFoundation::hash_combine(hash, options.viewCount);
        KDFoundation::hash_combine(hash, options.dynamicState);
        KDFoundation::hash_combine(hash, options.renderPass);
        KDFoundation::hash_combine(hash, options.subpassIndex);
        return hash;
    }
};

} // namespace std
