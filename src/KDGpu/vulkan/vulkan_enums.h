/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/gpu_core.h>
#include <KDGpu/kdgpu_export.h>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace KDGpu {

AdapterDeviceType vkPhysicalDeviceTypeToAdapterDeviceType(VkPhysicalDeviceType deviceType);

KDGPU_EXPORT Format vkFormatToFormat(VkFormat format);
VkFormat formatToVkFormat(Format format);

VkSampleCountFlagBits sampleCountFlagBitsToVkSampleFlagBits(SampleCountFlagBits samples);
SampleCountFlagBits vkSampleCountFlagBitsToSampleFlagBits(VkSampleCountFlagBits samples);

ColorSpace vkColorSpaceKHRToColorSpace(VkColorSpaceKHR colorSpace);
VkColorSpaceKHR colorSpaceToVkColorSpaceKHR(ColorSpace colorSpace);

PresentMode vkPresentModeKHRToPresentMode(VkPresentModeKHR presentMode);
VkPresentModeKHR presentModeToVkPresentModeKHR(PresentMode presentMode);

SurfaceTransformFlagBits vkSurfaceTransformFlagBitsKHRToSurfaceTransformFlagBits(VkSurfaceTransformFlagBitsKHR transform);
VkSurfaceTransformFlagBitsKHR surfaceTransformFlagBitsToVkSurfaceTransformFlagBitsKHR(SurfaceTransformFlagBits transform);

CompositeAlphaFlagBits vkCompositeAlphaFlagBitsKHRToCompositeAlphaFlagBits(VkCompositeAlphaFlagBitsKHR compositeAlpha);
VkCompositeAlphaFlagBitsKHR compositeAlphaFlagBitsToVkCompositeAlphaFlagBitsKHR(CompositeAlphaFlagBits compositeAlpha);

SharingMode vkSharingModeToSharingMode(VkSharingMode sharingMode);
VkSharingMode sharingModeToVkSharingMode(SharingMode sharingMode);

VkImageType textureTypeToVkImageType(TextureType textureType);
TextureType vkImageTypeToTextureType(VkImageType textureType);

VkImageTiling textureTilingToVkImageTiling(TextureTiling tiling);
TextureTiling vkImageTilingToTextureTiling(VkImageTiling tiling);

VkImageLayout textureLayoutToVkImageLayout(TextureLayout layout);
TextureLayout vkImageLayoutToTextureLayout(VkImageLayout layout);

ViewType vkImageViewTypeToViewType(VkImageViewType viewType);
VkImageViewType viewTypeToVkImageViewType(ViewType viewType);

MemoryUsage vmaMemoryUsageToMemoryUsage(VmaMemoryUsage memoryUsage);
VmaMemoryUsage memoryUsageToVmaMemoryUsage(MemoryUsage memoryUsage);

VkDescriptorType resourceBindingTypeToVkDescriptorType(ResourceBindingType type);
ResourceBindingType vkDescriptorTypeToVkDescriptorType(VkDescriptorType type);

VkShaderStageFlagBits shaderStageFlagBitsToVkShaderStageFlagBits(ShaderStageFlagBits stage);
ShaderStageFlagBits vkShaderStageFlagBitsToShaderStageFlagBits(VkShaderStageFlagBits stage);

VkVertexInputRate vertexRateToVkVertexInputRate(VertexRate rate);
VertexRate vkVertexInputRateToVertexRate(VkVertexInputRate rate);

VkPrimitiveTopology primitiveTopologyToVkPrimitiveTopology(PrimitiveTopology topology);
PrimitiveTopology vkPrimitiveTopologyToPrimitiveTopology(VkPrimitiveTopology topology);

VkPolygonMode polygonModeToVkPolygonMode(PolygonMode mode);
PolygonMode vkPolygonModeToPolygonMode(VkPolygonMode mode);

VkFrontFace frontFaceToVkFrontFace(FrontFace face);
FrontFace vkFrontFaceToFrontFace(VkFrontFace face);

VkCompareOp compareOperationToVkCompareOp(CompareOperation op);
CompareOperation vkCompareOpToCompareOperation(VkCompareOp op);

VkStencilOp stencilOperationToVkStencilOp(StencilOperation op);
StencilOperation vkStencilOpToStencilOperation(StencilOperation op);

VkBlendFactor blendFactorToVkBlendFactor(BlendFactor factor);
BlendFactor vkBlendFactorToBlendFactor(VkBlendFactor factor);

VkBlendOp blendOperationToVkBlendOp(BlendOperation op);
BlendOperation vkBlendOpToBlendOperation(BlendOperation op);

VkAttachmentLoadOp attachmentLoadOperationToVkAttachmentLoadOp(AttachmentLoadOperation op);
AttachmentLoadOperation vkAttachmentLoadOperationToAttachmentLoadOperation(AttachmentLoadOperation op);

VkAttachmentStoreOp attachmentStoreOperationToVkAttachmentStoreOp(AttachmentStoreOperation op);
AttachmentStoreOperation vkAttachmentStoreOperationToAttachmentStoreOperation(AttachmentStoreOperation op);

VkFilter filterModeToVkFilterMode(FilterMode mode);
VkSamplerMipmapMode mipMapFilterModeToVkSamplerMipmapMode(MipmapFilterMode mode);
VkSamplerAddressMode addressModeToVkSamplerAddressMode(AddressMode mode);

// TODO: Support VkAccessFlags2 features
VkAccessFlagBits accessFlagsToVkAccessFlagBits(AccessFlags accessFlags);
VkAccessFlagBits2KHR accessFlagsToVkAccessFlagBits2(AccessFlags accessFlags);

VkPipelineStageFlagBits pipelineStageFlagsToVkPipelineStageFlagBits(PipelineStageFlags pipelineFlags);
VkPipelineStageFlagBits2KHR pipelineStageFlagsToVkPipelineStageFlagBits2(PipelineStageFlags pipelineFlags);

VkIndexType indexTypeToVkIndexType(IndexType type);
IndexType vkIndexTypeToIndexType(VkIndexType type);

VkCommandBufferLevel commandBufferLevelToVkCommandBufferLevel(CommandBufferLevel level);

} // namespace KDGpu
