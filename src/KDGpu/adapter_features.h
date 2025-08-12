/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

namespace KDGpu {

/*! \addtogroup public
 *  @{
 */

/**
    @headerfile adapter_features.h <KDGpu/adapter_features.h>
 */

struct AdapterFeatures {
    bool robustBufferAccess;
    bool fullDrawIndexUint32;
    bool imageCubeArray;
    bool independentBlend;
    bool geometryShader;
    bool tessellationShader;
    bool sampleRateShading;
    bool dualSrcBlend;
    bool logicOp;
    bool multiDrawIndirect;
    bool drawIndirectFirstInstance;
    bool depthClamp;
    bool depthBiasClamp;
    bool fillModeNonSolid;
    bool depthBounds;
    bool wideLines;
    bool largePoints;
    bool alphaToOne;
    bool multiViewport;
    bool samplerAnisotropy;
    bool textureCompressionETC2;
    bool textureCompressionASTC_LDR;
    bool textureCompressionBC;
    bool occlusionQueryPrecise;
    bool pipelineStatisticsQuery;
    bool vertexPipelineStoresAndAtomics;
    bool fragmentStoresAndAtomics;
    bool shaderTessellationAndGeometryPointSize;
    bool shaderImageGatherExtended;
    bool shaderStorageImageExtendedFormats;
    bool shaderStorageImageMultisample;
    bool shaderStorageImageReadWithoutFormat;
    bool shaderStorageImageWriteWithoutFormat;
    bool shaderUniformBufferArrayDynamicIndexing;
    bool shaderSampledImageArrayDynamicIndexing;
    bool shaderStorageBufferArrayDynamicIndexing;
    bool shaderStorageImageArrayDynamicIndexing;
    bool shaderClipDistance;
    bool shaderCullDistance;
    bool shaderFloat64;
    bool shaderInt64;
    bool shaderInt16;
    bool shaderResourceResidency;
    bool shaderResourceMinLod;
    bool sparseBinding;
    bool sparseResidencyBuffer;
    bool sparseResidencyImage2D;
    bool sparseResidencyImage3D;
    bool sparseResidency2Samples;
    bool sparseResidency4Samples;
    bool sparseResidency8Samples;
    bool sparseResidency16Samples;
    bool sparseResidencyAliased;
    bool variableMultisampleRate;
    bool inheritedQueries;
    bool uniformBufferStandardLayout;
    bool multiView;
    bool multiViewGeometryShader;
    bool multiViewTessellationShader;
    bool shaderInputAttachmentArrayDynamicIndexing;
    bool shaderUniformTexelBufferArrayDynamicIndexing;
    bool shaderStorageTexelBufferArrayDynamicIndexing;
    bool shaderUniformBufferArrayNonUniformIndexing;
    bool shaderSampledImageArrayNonUniformIndexing;
    bool shaderStorageBufferArrayNonUniformIndexing;
    bool shaderStorageImageArrayNonUniformIndexing;
    bool shaderInputAttachmentArrayNonUniformIndexing;
    bool shaderUniformTexelBufferArrayNonUniformIndexing;
    bool shaderStorageTexelBufferArrayNonUniformIndexing;
    bool bindGroupBindingUniformBufferUpdateAfterBind;
    bool bindGroupBindingSampledImageUpdateAfterBind;
    bool bindGroupBindingStorageImageUpdateAfterBind;
    bool bindGroupBindingStorageBufferUpdateAfterBind;
    bool bindGroupBindingUniformTexelBufferUpdateAfterBind;
    bool bindGroupBindingStorageTexelBufferUpdateAfterBind;
    bool bindGroupBindingUpdateUnusedWhilePending;
    bool bindGroupBindingPartiallyBound;
    bool bindGroupBindingVariableDescriptorCount;
    bool runtimeBindGroupArray;
    bool bufferDeviceAddress;
    bool accelerationStructures;
    bool rayTracingPipeline;
    bool rayTracingPipelineShaderGroupHandleCaptureReplay;
    bool rayTracingPipelineShaderGroupHandleCaptureReplayMixed;
    bool rayTracingPipelineTraceRaysIndirect;
    bool rayTraversalPrimitiveCulling;
    bool taskShader;
    bool meshShader;
    bool multiviewMeshShader;
    bool primitiveFragmentShadingRateMeshShader;
    bool meshShaderQueries;
    bool hostImageCopy;
    bool samplerYCbCrConversion;
    bool dynamicRendering;
    bool dynamicRenderingLocalRead;
};

/*! @} */

} // namespace KDGpu
