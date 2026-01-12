/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/gpu_core.h>

#include <array>
#include <stdint.h>
#include <string>
#include <vector>

namespace KDGpu {

/*! \addtogroup public
 *  @{
 */

/**
    @headerfile adapter_properties.h <KDGpu/adapter_properties.h>
 */
struct AdapterLimits {
    uint32_t maxImageDimension1D{ 0 };
    uint32_t maxImageDimension2D{ 0 };
    uint32_t maxImageDimension3D{ 0 };
    uint32_t maxImageDimensionCube{ 0 };
    uint32_t maxImageArrayLayers{ 0 };
    uint32_t maxTexelBufferElements{ 0 };
    uint32_t maxUniformBufferRange{ 0 };
    uint32_t maxStorageBufferRange{ 0 };
    uint32_t maxPushConstantsSize{ 0 };
    uint32_t maxMemoryAllocationCount{ 0 };
    uint32_t maxSamplerAllocationCount{ 0 };
    DeviceSize bufferImageGranularity{ 0 };
    DeviceSize sparseAddressSpaceSize{ 0 };
    uint32_t maxBoundDescriptorSets{ 0 };
    uint32_t maxPerStageDescriptorSamplers{ 0 };
    uint32_t maxPerStageDescriptorUniformBuffers{ 0 };
    uint32_t maxPerStageDescriptorStorageBuffers{ 0 };
    uint32_t maxPerStageDescriptorSampledImages{ 0 };
    uint32_t maxPerStageDescriptorStorageImages{ 0 };
    uint32_t maxPerStageDescriptorInputAttachments{ 0 };
    uint32_t maxPerStageResources{ 0 };
    uint32_t maxDescriptorSetSamplers{ 0 };
    uint32_t maxDescriptorSetUniformBuffers{ 0 };
    uint32_t maxDescriptorSetUniformBuffersDynamic{ 0 };
    uint32_t maxDescriptorSetStorageBuffers{ 0 };
    uint32_t maxDescriptorSetStorageBuffersDynamic{ 0 };
    uint32_t maxDescriptorSetSampledImages{ 0 };
    uint32_t maxDescriptorSetStorageImages{ 0 };
    uint32_t maxDescriptorSetInputAttachments{ 0 };
    uint32_t maxVertexInputAttributes{ 0 };
    uint32_t maxVertexInputBindings{ 0 };
    uint32_t maxVertexInputAttributeOffset{ 0 };
    uint32_t maxVertexInputBindingStride{ 0 };
    uint32_t maxVertexOutputComponents{ 0 };
    uint32_t maxTessellationGenerationLevel{ 0 };
    uint32_t maxTessellationPatchSize{ 0 };
    uint32_t maxTessellationControlPerVertexInputComponents{ 0 };
    uint32_t maxTessellationControlPerVertexOutputComponents{ 0 };
    uint32_t maxTessellationControlPerPatchOutputComponents{ 0 };
    uint32_t maxTessellationControlTotalOutputComponents{ 0 };
    uint32_t maxTessellationEvaluationInputComponents{ 0 };
    uint32_t maxTessellationEvaluationOutputComponents{ 0 };
    uint32_t maxGeometryShaderInvocations{ 0 };
    uint32_t maxGeometryInputComponents{ 0 };
    uint32_t maxGeometryOutputComponents{ 0 };
    uint32_t maxGeometryOutputVertices{ 0 };
    uint32_t maxGeometryTotalOutputComponents{ 0 };
    uint32_t maxFragmentInputComponents{ 0 };
    uint32_t maxFragmentOutputAttachments{ 0 };
    uint32_t maxFragmentDualSrcAttachments{ 0 };
    uint32_t maxFragmentCombinedOutputResources{ 0 };
    uint32_t maxComputeSharedMemorySize{ 0 };
    std::array<uint32_t, 3> maxComputeWorkGroupCount{ 0, 0, 0 };
    uint32_t maxComputeWorkGroupInvocations{ 0 };
    std::array<uint32_t, 3> maxComputeWorkGroupSize{ 0, 0, 0 };
    uint32_t subPixelPrecisionBits{ 0 };
    uint32_t subTexelPrecisionBits{ 0 };
    uint32_t mipmapPrecisionBits{ 0 };
    uint32_t maxDrawIndexedIndexValue{ 0 };
    uint32_t maxDrawIndirectCount{ 0 };
    float maxSamplerLodBias{ 0.0f };
    float maxSamplerAnisotropy{ 0.0f };
    uint32_t maxViewports{ 0 };
    std::array<uint32_t, 2> maxViewportDimensions{ 0, 0 };
    std::array<float, 2> viewportBoundsRange{ 0.0f, 0.0f };
    uint32_t viewportSubPixelBits{ 0 };
    size_t minMemoryMapAlignment{ 0 };
    DeviceSize minTexelBufferOffsetAlignment{ 0 };
    DeviceSize minUniformBufferOffsetAlignment{ 0 };
    DeviceSize minStorageBufferOffsetAlignment{ 0 };
    int32_t minTexelOffset{ 0 };
    uint32_t maxTexelOffset{ 0 };
    int32_t minTexelGatherOffset{ 0 };
    uint32_t maxTexelGatherOffset{ 0 };
    float minInterpolationOffset{ 0.0f };
    float maxInterpolationOffset{ 0.0f };
    uint32_t subPixelInterpolationOffsetBits{ 0 };
    uint32_t maxFramebufferWidth{ 0 };
    uint32_t maxFramebufferHeight{ 0 };
    uint32_t maxFramebufferLayers{ 0 };
    SampleCountFlags framebufferColorSampleCounts;
    SampleCountFlags framebufferDepthSampleCounts;
    SampleCountFlags framebufferStencilSampleCounts;
    SampleCountFlags framebufferNoAttachmentsSampleCounts;
    uint32_t maxColorAttachments{ 0 };
    SampleCountFlags sampledImageColorSampleCounts;
    SampleCountFlags sampledImageIntegerSampleCounts;
    SampleCountFlags sampledImageDepthSampleCounts;
    SampleCountFlags sampledImageStencilSampleCounts;
    SampleCountFlags storageImageSampleCounts;
    uint32_t maxSampleMaskWords{ 0 };
    bool timestampComputeAndGraphics{ false };
    float timestampPeriod{ 0.0f };
    uint32_t maxClipDistances{ 0 };
    uint32_t maxCullDistances{ 0 };
    uint32_t maxCombinedClipAndCullDistances{ 0 };
    uint32_t discreteQueuePriorities{ 0 };
    std::array<float, 2> pointSizeRange{ 0.0f, 0.0f };
    std::array<float, 2> lineWidthRange{ 0.0f, 0.0f };
    float pointSizeGranularity{ 0.0f };
    float lineWidthGranularity{ 0.0f };
    bool strictLines{ false };
    bool standardSampleLocations{ false };
    DeviceSize optimalBufferCopyOffsetAlignment{ 0 };
    DeviceSize optimalBufferCopyRowPitchAlignment{ 0 };
    DeviceSize nonCoherentAtomSize{ 0 };
};

/**
    @headerfile adapter_properties.h <KDGpu/adapter_properties.h>
 */
struct AdapterSparseProperties {
    bool residencyStandard2DBlockShape{ false };
    bool residencyStandard2DMultisampleBlockShape{ false };
    bool residencyStandard3DBlockShape{ false };
    bool residencyAlignedMipSize{ false };
    bool residencyNonResidentStrict{ false };
};

/**
    @headerfile adapter_properties.h <KDGpu/adapter_properties.h>
 */
struct AdapterMultiViewProperties {
    uint32_t maxMultiViewCount{ 0 };
    uint32_t maxMultiviewInstanceIndex{ 0 };
};

/**
    @headerfile adapter_properties.h <KDGpu/adapter_properties.h>
 */
struct AdapterDepthStencilResolveProperties {
    ResolveModeFlags supportedDepthResolveModes;
    ResolveModeFlags supportedStencilResolveModes;
    bool independentResolveNone{ false };
    bool independentResolve{ false };
};

/**
    @headerfile adapter_properties.h <KDGpu/adapter_properties.h>
 */
struct BindGroupIndexingProperties {
    uint32_t maxUpdateAfterBindBindGroups{ 0 };
    bool shaderUniformBufferArrayNonUniformIndexingNative{ false };
    bool shaderSampledImageArrayNonUniformIndexingNative{ false };
    bool shaderStorageBufferArrayNonUniformIndexingNative{ false };
    bool shaderStorageImageArrayNonUniformIndexingNative{ false };
    bool shaderInputAttachmentArrayNonUniformIndexingNative{ false };
    bool robustBufferAccessUpdateAfterBind{ false };
    bool quadDivergentImplicitLod{ false };
    uint32_t maxPerStageBindGroupEntriesUpdateAfterBindSamplers{ 0 };
    uint32_t maxPerStageBindGroupEntriesUpdateAfterBindUniformBuffers{ 0 };
    uint32_t maxPerStageBindGroupEntriesUpdateAfterBindStorageBuffers{ 0 };
    uint32_t maxPerStageBindGroupEntriesUpdateAfterBindSampledImages{ 0 };
    uint32_t maxPerStageBindGroupEntriesUpdateAfterBindStorageImages{ 0 };
    uint32_t maxPerStageBindGroupEntriesUpdateAfterBindInputAttachments{ 0 };
    uint32_t maxPerStageUpdateAfterBindResources{ 0 };
    uint32_t maxBindGroupUpdateAfterBindSamplers{ 0 };
    uint32_t maxBindGroupUpdateAfterBindUniformBuffers{ 0 };
    uint32_t maxBindGroupUpdateAfterBindUniformBuffersDynamic{ 0 };
    uint32_t maxBindGroupUpdateAfterBindStorageBuffers{ 0 };
    uint32_t maxBindGroupUpdateAfterBindStorageBuffersDynamic{ 0 };
    uint32_t maxBindGroupUpdateAfterBindSampledImages{ 0 };
    uint32_t maxBindGroupUpdateAfterBindStorageImages{ 0 };
    uint32_t maxBindGroupUpdateAfterBindInputAttachments{ 0 };
};

/**
    @headerfile adapter_properties.h <KDGpu/adapter_properties.h>
 */
struct RayTracingProperties {
    uint32_t shaderGroupHandleSize{ 0 };
    uint32_t maxRayRecursionDepth{ 0 };
    uint32_t maxShaderGroupStride{ 0 };
    uint32_t shaderGroupBaseAlignment{ 0 };
    uint32_t shaderGroupHandleCaptureReplaySize{ 0 };
    uint32_t maxRayDispatchInvocationCount{ 0 };
    uint32_t shaderGroupHandleAlignment{ 0 };
    uint32_t maxRayHitAttributeSize{ 0 };
};

/**
    @headerfile adapter_properties.h <KDGpu/adapter_properties.h>
 */
struct MeshShaderProperties {
    uint32_t maxTaskWorkGroupTotalCount{ 0 };
    std::array<uint32_t, 3> maxTaskWorkGroupCount{ 0, 0, 0 };
    uint32_t maxTaskWorkGroupInvocations{ 0 };
    std::array<uint32_t, 3> maxTaskWorkGroupSize{ 0, 0, 0 };
    uint32_t maxTaskPayloadSize{ 0 };
    uint32_t maxTaskSharedMemorySize{ 0 };
    uint32_t maxTaskPayloadAndSharedMemorySize{ 0 };
    uint32_t maxMeshWorkGroupTotalCount{ 0 };
    std::array<uint32_t, 3> maxMeshWorkGroupCount{ 0, 0, 0 };
    uint32_t maxMeshWorkGroupInvocations{ 0 };
    std::array<uint32_t, 3> maxMeshWorkGroupSize{ 0, 0, 0 };
    uint32_t maxMeshSharedMemorySize{ 0 };
    uint32_t maxMeshPayloadAndSharedMemorySize{ 0 };
    uint32_t maxMeshOutputMemorySize{ 0 };
    uint32_t maxMeshPayloadAndOutputMemorySize{ 0 };
    uint32_t maxMeshOutputComponents{ 0 };
    uint32_t maxMeshOutputVertices{ 0 };
    uint32_t maxMeshOutputPrimitives{ 0 };
    uint32_t maxMeshOutputLayers{ 0 };
    uint32_t maxMeshMultiviewViewCount{ 0 };
    uint32_t meshOutputPerVertexGranularity{ 0 };
    uint32_t meshOutputPerPrimitiveGranularity{ 0 };
    uint32_t maxPreferredTaskWorkGroupInvocations{ 0 };
    uint32_t maxPreferredMeshWorkGroupInvocations{ 0 };
    bool prefersLocalInvocationVertexOutput{ false };
    bool prefersLocalInvocationPrimitiveOutput{ false };
    bool prefersCompactVertexOutput{ false };
    bool prefersCompactPrimitiveOutput{ false };
};

/**
    @headerfile adapter_properties.h <KDGpu/adapter_properties.h>
 */
struct HostImageCopyProperties {
    std::vector<TextureLayout> srcCopyLayouts;
    std::vector<TextureLayout> dstCopyLayouts;
};

/**
    @headerfile adapter_properties.h <KDGpu/adapter_properties.h>
 */
struct PushBindGroupProperties {
    uint32_t maxPushBindGroups{ 0 };
};

/**
    @headerfile adapter_properties.h <KDGpu/adapter_properties.h>
 */
struct AdapterProperties {
    uint32_t apiVersion{ 0 };
    uint32_t driverVersion{ 0 };
    uint32_t vendorID{ 0 };
    uint32_t deviceID{ 0 };
    AdapterDeviceType deviceType;
    std::string deviceName;
    std::array<uint8_t, UuidSize> pipelineCacheUUID{};
    AdapterLimits limits;
    AdapterSparseProperties sparseProperties;
    AdapterMultiViewProperties multiViewProperties;
    AdapterDepthStencilResolveProperties depthResolveProperties;
    BindGroupIndexingProperties bindGroupIndexingProperties;
    RayTracingProperties rayTracingProperties;
    MeshShaderProperties meshShaderProperties;
    HostImageCopyProperties hostImageCopyProperties;
    PushBindGroupProperties pushBindGroupProperties;
};

/**
    @headerfile adapter_properties.h <KDGpu/adapter_properties.h>
 */
struct FormatProperties {
    FormatFeatureFlags linearTilingFeatures;
    FormatFeatureFlags optimalTilingFeatures;
    FormatFeatureFlags bufferFeatures;
};

/**
    @headerfile adapter_properties.h <KDGpu/adapter_properties.h>
 */
struct DrmFormatModifierProperties {
    uint64_t formatModifier{ 0 };
    uint32_t planeCount{ 0 };
    FormatFeatureFlags featureFlags;
};

/*! @} */

} // namespace KDGpu
