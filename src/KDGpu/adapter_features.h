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
    bool robustBufferAccess{ false };
    bool fullDrawIndexUint32{ false };
    bool imageCubeArray{ false };
    bool independentBlend{ false };
    bool geometryShader{ false };
    bool tessellationShader{ false };
    bool sampleRateShading{ false };
    bool dualSrcBlend{ false };
    bool logicOp{ false };
    bool multiDrawIndirect{ false };
    bool drawIndirectFirstInstance{ false };
    bool depthClamp{ false };
    bool depthBiasClamp{ false };
    bool fillModeNonSolid{ false };
    bool depthBounds{ false };
    bool wideLines{ false };
    bool largePoints{ false };
    bool alphaToOne{ false };
    bool multiViewport{ false };
    bool samplerAnisotropy{ false };
    bool textureCompressionETC2{ false };
    bool textureCompressionASTC_LDR{ false };
    bool textureCompressionBC{ false };
    bool occlusionQueryPrecise{ false };
    bool pipelineStatisticsQuery{ false };
    bool vertexPipelineStoresAndAtomics{ false };
    bool fragmentStoresAndAtomics{ false };
    bool shaderTessellationAndGeometryPointSize{ false };
    bool shaderImageGatherExtended{ false };
    bool shaderStorageImageExtendedFormats{ false };
    bool shaderStorageImageMultisample{ false };
    bool shaderStorageImageReadWithoutFormat{ false };
    bool shaderStorageImageWriteWithoutFormat{ false };
    bool shaderUniformBufferArrayDynamicIndexing{ false };
    bool shaderSampledImageArrayDynamicIndexing{ false };
    bool shaderStorageBufferArrayDynamicIndexing{ false };
    bool shaderStorageImageArrayDynamicIndexing{ false };
    bool shaderClipDistance{ false };
    bool shaderCullDistance{ false };
    bool shaderFloat64{ false };
    bool shaderInt64{ false };
    bool shaderInt16{ false };
    bool shaderResourceResidency{ false };
    bool shaderResourceMinLod{ false };
    bool sparseBinding{ false };
    bool sparseResidencyBuffer{ false };
    bool sparseResidencyImage2D{ false };
    bool sparseResidencyImage3D{ false };
    bool sparseResidency2Samples{ false };
    bool sparseResidency4Samples{ false };
    bool sparseResidency8Samples{ false };
    bool sparseResidency16Samples{ false };
    bool sparseResidencyAliased{ false };
    bool variableMultisampleRate{ false };
    bool inheritedQueries{ false };
    bool uniformBufferStandardLayout{ false };
    bool multiView{ false };
    bool multiViewGeometryShader{ false };
    bool multiViewTessellationShader{ false };
    bool shaderInputAttachmentArrayDynamicIndexing{ false };
    bool shaderUniformTexelBufferArrayDynamicIndexing{ false };
    bool shaderStorageTexelBufferArrayDynamicIndexing{ false };
    bool shaderUniformBufferArrayNonUniformIndexing{ false };
    bool shaderSampledImageArrayNonUniformIndexing{ false };
    bool shaderStorageBufferArrayNonUniformIndexing{ false };
    bool shaderStorageImageArrayNonUniformIndexing{ false };
    bool shaderInputAttachmentArrayNonUniformIndexing{ false };
    bool shaderUniformTexelBufferArrayNonUniformIndexing{ false };
    bool shaderStorageTexelBufferArrayNonUniformIndexing{ false };
    bool bindGroupBindingUniformBufferUpdateAfterBind{ false };
    bool bindGroupBindingSampledImageUpdateAfterBind{ false };
    bool bindGroupBindingStorageImageUpdateAfterBind{ false };
    bool bindGroupBindingStorageBufferUpdateAfterBind{ false };
    bool bindGroupBindingUniformTexelBufferUpdateAfterBind{ false };
    bool bindGroupBindingStorageTexelBufferUpdateAfterBind{ false };
    bool bindGroupBindingUpdateUnusedWhilePending{ false };
    bool bindGroupBindingPartiallyBound{ false };
    bool bindGroupBindingVariableDescriptorCount{ false };
    bool runtimeBindGroupArray{ false };
    bool bufferDeviceAddress{ false };
    bool pipelineFragmentShadingRate{ false };
    bool primitiveFragmentShadingRate{ false };
    bool attachmentFragmentShadingRate{ false };
    bool accelerationStructures{ false };
    bool rayTracingPipeline{ false };
    bool rayTracingPipelineShaderGroupHandleCaptureReplay{ false };
    bool rayTracingPipelineShaderGroupHandleCaptureReplayMixed{ false };
    bool rayTracingPipelineTraceRaysIndirect{ false };
    bool rayTraversalPrimitiveCulling{ false };
    bool taskShader{ false };
    bool meshShader{ false };
    bool multiviewMeshShader{ false };
    bool primitiveFragmentShadingRateMeshShader{ false };
    bool meshShaderQueries{ false };
    bool hostImageCopy{ false };
    bool samplerYCbCrConversion{ false };
    bool dynamicRendering{ false };
    bool dynamicRenderingLocalRead{ false };
};

/*! @} */

} // namespace KDGpu
