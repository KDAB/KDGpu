/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/gpu_core.h>

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
    uint32_t maxImageDimension1D;
    uint32_t maxImageDimension2D;
    uint32_t maxImageDimension3D;
    uint32_t maxImageDimensionCube;
    uint32_t maxImageArrayLayers;
    uint32_t maxTexelBufferElements;
    uint32_t maxUniformBufferRange;
    uint32_t maxStorageBufferRange;
    uint32_t maxPushConstantsSize;
    uint32_t maxMemoryAllocationCount;
    uint32_t maxSamplerAllocationCount;
    DeviceSize bufferImageGranularity;
    DeviceSize sparseAddressSpaceSize;
    uint32_t maxBoundDescriptorSets;
    uint32_t maxPerStageDescriptorSamplers;
    uint32_t maxPerStageDescriptorUniformBuffers;
    uint32_t maxPerStageDescriptorStorageBuffers;
    uint32_t maxPerStageDescriptorSampledImages;
    uint32_t maxPerStageDescriptorStorageImages;
    uint32_t maxPerStageDescriptorInputAttachments;
    uint32_t maxPerStageResources;
    uint32_t maxDescriptorSetSamplers;
    uint32_t maxDescriptorSetUniformBuffers;
    uint32_t maxDescriptorSetUniformBuffersDynamic;
    uint32_t maxDescriptorSetStorageBuffers;
    uint32_t maxDescriptorSetStorageBuffersDynamic;
    uint32_t maxDescriptorSetSampledImages;
    uint32_t maxDescriptorSetStorageImages;
    uint32_t maxDescriptorSetInputAttachments;
    uint32_t maxVertexInputAttributes;
    uint32_t maxVertexInputBindings;
    uint32_t maxVertexInputAttributeOffset;
    uint32_t maxVertexInputBindingStride;
    uint32_t maxVertexOutputComponents;
    uint32_t maxTessellationGenerationLevel;
    uint32_t maxTessellationPatchSize;
    uint32_t maxTessellationControlPerVertexInputComponents;
    uint32_t maxTessellationControlPerVertexOutputComponents;
    uint32_t maxTessellationControlPerPatchOutputComponents;
    uint32_t maxTessellationControlTotalOutputComponents;
    uint32_t maxTessellationEvaluationInputComponents;
    uint32_t maxTessellationEvaluationOutputComponents;
    uint32_t maxGeometryShaderInvocations;
    uint32_t maxGeometryInputComponents;
    uint32_t maxGeometryOutputComponents;
    uint32_t maxGeometryOutputVertices;
    uint32_t maxGeometryTotalOutputComponents;
    uint32_t maxFragmentInputComponents;
    uint32_t maxFragmentOutputAttachments;
    uint32_t maxFragmentDualSrcAttachments;
    uint32_t maxFragmentCombinedOutputResources;
    uint32_t maxComputeSharedMemorySize;
    uint32_t maxComputeWorkGroupCount[3];
    uint32_t maxComputeWorkGroupInvocations;
    uint32_t maxComputeWorkGroupSize[3];
    uint32_t subPixelPrecisionBits;
    uint32_t subTexelPrecisionBits;
    uint32_t mipmapPrecisionBits;
    uint32_t maxDrawIndexedIndexValue;
    uint32_t maxDrawIndirectCount;
    float maxSamplerLodBias;
    float maxSamplerAnisotropy;
    uint32_t maxViewports;
    uint32_t maxViewportDimensions[2];
    float viewportBoundsRange[2];
    uint32_t viewportSubPixelBits;
    size_t minMemoryMapAlignment;
    DeviceSize minTexelBufferOffsetAlignment;
    DeviceSize minUniformBufferOffsetAlignment;
    DeviceSize minStorageBufferOffsetAlignment;
    int32_t minTexelOffset;
    uint32_t maxTexelOffset;
    int32_t minTexelGatherOffset;
    uint32_t maxTexelGatherOffset;
    float minInterpolationOffset;
    float maxInterpolationOffset;
    uint32_t subPixelInterpolationOffsetBits;
    uint32_t maxFramebufferWidth;
    uint32_t maxFramebufferHeight;
    uint32_t maxFramebufferLayers;
    SampleCountFlags framebufferColorSampleCounts;
    SampleCountFlags framebufferDepthSampleCounts;
    SampleCountFlags framebufferStencilSampleCounts;
    SampleCountFlags framebufferNoAttachmentsSampleCounts;
    uint32_t maxColorAttachments;
    SampleCountFlags sampledImageColorSampleCounts;
    SampleCountFlags sampledImageIntegerSampleCounts;
    SampleCountFlags sampledImageDepthSampleCounts;
    SampleCountFlags sampledImageStencilSampleCounts;
    SampleCountFlags storageImageSampleCounts;
    uint32_t maxSampleMaskWords;
    bool timestampComputeAndGraphics;
    float timestampPeriod;
    uint32_t maxClipDistances;
    uint32_t maxCullDistances;
    uint32_t maxCombinedClipAndCullDistances;
    uint32_t discreteQueuePriorities;
    float pointSizeRange[2];
    float lineWidthRange[2];
    float pointSizeGranularity;
    float lineWidthGranularity;
    bool strictLines;
    bool standardSampleLocations;
    DeviceSize optimalBufferCopyOffsetAlignment;
    DeviceSize optimalBufferCopyRowPitchAlignment;
    DeviceSize nonCoherentAtomSize;
};

/**
    @headerfile adapter_properties.h <KDGpu/adapter_properties.h>
 */
struct AdapterSparseProperties {
    bool residencyStandard2DBlockShape;
    bool residencyStandard2DMultisampleBlockShape;
    bool residencyStandard3DBlockShape;
    bool residencyAlignedMipSize;
    bool residencyNonResidentStrict;
};

/**
    @headerfile adapter_properties.h <KDGpu/adapter_properties.h>
 */
struct AdapterMultiViewProperties {
    uint32_t maxMultiViewCount;
    uint32_t maxMultiviewInstanceIndex;
};

/**
    @headerfile adapter_properties.h <KDGpu/adapter_properties.h>
 */
struct AdapterDepthStencilResolveProperties {
    ResolveModeFlags supportedDepthResolveModes;
    ResolveModeFlags supportedStencilResolveModes;
    bool independentResolveNone;
    bool independentResolve;
};

/**
    @headerfile adapter_properties.h <KDGpu/adapter_properties.h>
 */
struct BindGroupIndexingProperties {
    uint32_t maxUpdateAfterBindBindGroups;
    bool shaderUniformBufferArrayNonUniformIndexingNative;
    bool shaderSampledImageArrayNonUniformIndexingNative;
    bool shaderStorageBufferArrayNonUniformIndexingNative;
    bool shaderStorageImageArrayNonUniformIndexingNative;
    bool shaderInputAttachmentArrayNonUniformIndexingNative;
    bool robustBufferAccessUpdateAfterBind;
    bool quadDivergentImplicitLod;
    uint32_t maxPerStageBindGroupEntriesUpdateAfterBindSamplers;
    uint32_t maxPerStageBindGroupEntriesUpdateAfterBindUniformBuffers;
    uint32_t maxPerStageBindGroupEntriesUpdateAfterBindStorageBuffers;
    uint32_t maxPerStageBindGroupEntriesUpdateAfterBindSampledImages;
    uint32_t maxPerStageBindGroupEntriesUpdateAfterBindStorageImages;
    uint32_t maxPerStageBindGroupEntriesUpdateAfterBindInputAttachments;
    uint32_t maxPerStageUpdateAfterBindResources;
    uint32_t maxBindGroupUpdateAfterBindSamplers;
    uint32_t maxBindGroupUpdateAfterBindUniformBuffers;
    uint32_t maxBindGroupUpdateAfterBindUniformBuffersDynamic;
    uint32_t maxBindGroupUpdateAfterBindStorageBuffers;
    uint32_t maxBindGroupUpdateAfterBindStorageBuffersDynamic;
    uint32_t maxBindGroupUpdateAfterBindSampledImages;
    uint32_t maxBindGroupUpdateAfterBindStorageImages;
    uint32_t maxBindGroupUpdateAfterBindInputAttachments;
};

struct RayTracingProperties {
    uint32_t shaderGroupHandleSize;
    uint32_t maxRayRecursionDepth;
    uint32_t maxShaderGroupStride;
    uint32_t shaderGroupBaseAlignment;
    uint32_t shaderGroupHandleCaptureReplaySize;
    uint32_t maxRayDispatchInvocationCount;
    uint32_t shaderGroupHandleAlignment;
    uint32_t maxRayHitAttributeSize;
};

struct MeshShaderProperties {
    uint32_t maxTaskWorkGroupTotalCount;
    uint32_t maxTaskWorkGroupCount[3];
    uint32_t maxTaskWorkGroupInvocations;
    uint32_t maxTaskWorkGroupSize[3];
    uint32_t maxTaskPayloadSize;
    uint32_t maxTaskSharedMemorySize;
    uint32_t maxTaskPayloadAndSharedMemorySize;
    uint32_t maxMeshWorkGroupTotalCount;
    uint32_t maxMeshWorkGroupCount[3];
    uint32_t maxMeshWorkGroupInvocations;
    uint32_t maxMeshWorkGroupSize[3];
    uint32_t maxMeshSharedMemorySize;
    uint32_t maxMeshPayloadAndSharedMemorySize;
    uint32_t maxMeshOutputMemorySize;
    uint32_t maxMeshPayloadAndOutputMemorySize;
    uint32_t maxMeshOutputComponents;
    uint32_t maxMeshOutputVertices;
    uint32_t maxMeshOutputPrimitives;
    uint32_t maxMeshOutputLayers;
    uint32_t maxMeshMultiviewViewCount;
    uint32_t meshOutputPerVertexGranularity;
    uint32_t meshOutputPerPrimitiveGranularity;
    uint32_t maxPreferredTaskWorkGroupInvocations;
    uint32_t maxPreferredMeshWorkGroupInvocations;
    bool prefersLocalInvocationVertexOutput;
    bool prefersLocalInvocationPrimitiveOutput;
    bool prefersCompactVertexOutput;
    bool prefersCompactPrimitiveOutput;
};

struct HostImageCopyProperties {
    std::vector<TextureLayout> srcCopyLayouts;
    std::vector<TextureLayout> dstCopyLayouts;
};

struct PushBindGroupProperties {
    uint32_t maxPushBindGroups;
};

/**
    @headerfile adapter_properties.h <KDGpu/adapter_properties.h>
 */
struct AdapterProperties {
    uint32_t apiVersion;
    uint32_t driverVersion;
    uint32_t vendorID;
    uint32_t deviceID;
    AdapterDeviceType deviceType;
    std::string deviceName;
    uint8_t pipelineCacheUUID[UuidSize];
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
    uint64_t formatModifier;
    uint32_t planeCount;
    FormatFeatureFlags featureFlags;
};

/*! @} */

} // namespace KDGpu
