/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_enums.h"

#include <KDGpu/utils/logging.h>

namespace KDGpu {

AdapterDeviceType vkPhysicalDeviceTypeToAdapterDeviceType(VkPhysicalDeviceType deviceType)
{
    switch (deviceType) {
    case VK_PHYSICAL_DEVICE_TYPE_OTHER:
        return AdapterDeviceType::Other;

    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        return AdapterDeviceType::IntegratedGpu;

    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        return AdapterDeviceType::DiscreteGpu;

    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
        return AdapterDeviceType::VirtualGpu;

    case VK_PHYSICAL_DEVICE_TYPE_CPU:
        return AdapterDeviceType::Cpu;

    default:
        SPDLOG_LOGGER_WARN(Logger::logger(), "Unknown adapter type");
        return AdapterDeviceType::MaxEnum;
    }
}

Format vkFormatToFormat(VkFormat format)
{
    return static_cast<Format>(static_cast<uint32_t>(format));
}

VkFormat formatToVkFormat(Format format)
{
    return static_cast<VkFormat>(static_cast<uint32_t>(format));
}

VkSampleCountFlagBits sampleCountFlagBitsToVkSampleFlagBits(SampleCountFlagBits samples)
{
    return static_cast<VkSampleCountFlagBits>(static_cast<uint32_t>(samples));
}

SampleCountFlagBits vkSampleCountFlagBitsToSampleFlagBits(VkSampleCountFlagBits samples)
{
    return static_cast<SampleCountFlagBits>(static_cast<uint32_t>(samples));
}

ColorSpace vkColorSpaceKHRToColorSpace(VkColorSpaceKHR colorSpace)
{
    return static_cast<ColorSpace>(static_cast<uint32_t>(colorSpace));
}

VkColorSpaceKHR colorSpaceToVkColorSpaceKHR(ColorSpace colorSpace)
{
    return static_cast<VkColorSpaceKHR>(static_cast<uint32_t>(colorSpace));
}

PresentMode vkPresentModeKHRToPresentMode(VkPresentModeKHR presentMode)
{
    return static_cast<PresentMode>(static_cast<uint32_t>(presentMode));
}

VkPresentModeKHR presentModeToVkPresentModeKHR(PresentMode presentMode)
{
    return static_cast<VkPresentModeKHR>(static_cast<uint32_t>(presentMode));
}

SurfaceTransformFlagBits vkSurfaceTransformFlagBitsKHRToSurfaceTransformFlagBits(VkSurfaceTransformFlagBitsKHR transform)
{
    return static_cast<SurfaceTransformFlagBits>(static_cast<uint32_t>(transform));
}

VkSurfaceTransformFlagBitsKHR surfaceTransformFlagBitsToVkSurfaceTransformFlagBitsKHR(SurfaceTransformFlagBits transform)
{
    return static_cast<VkSurfaceTransformFlagBitsKHR>(static_cast<uint32_t>(transform));
}

CompositeAlphaFlagBits vkCompositeAlphaFlagBitsKHRToCompositeAlphaFlagBits(VkCompositeAlphaFlagBitsKHR compositeAlpha)
{
    return static_cast<CompositeAlphaFlagBits>(static_cast<uint32_t>(compositeAlpha));
}

VkCompositeAlphaFlagBitsKHR compositeAlphaFlagBitsToVkCompositeAlphaFlagBitsKHR(CompositeAlphaFlagBits compositeAlpha)
{
    return static_cast<VkCompositeAlphaFlagBitsKHR>(static_cast<uint32_t>(compositeAlpha));
}

SharingMode vkSharingModeToSharingMode(VkSharingMode sharingMode)
{
    return static_cast<SharingMode>(static_cast<uint32_t>(sharingMode));
}

VkSharingMode sharingModeToVkSharingMode(SharingMode sharingMode)
{
    return static_cast<VkSharingMode>(static_cast<uint32_t>(sharingMode));
}

VkImageType textureTypeToVkImageType(TextureType textureType)
{
    if (textureType == TextureType::TextureTypeCube)
        return VK_IMAGE_TYPE_2D;
    return static_cast<VkImageType>(static_cast<uint32_t>(textureType));
}

TextureType vkImageTypeToTextureType(VkImageType textureType)
{
    return static_cast<TextureType>(static_cast<uint32_t>(textureType));
}

VkImageTiling textureTilingToVkImageTiling(TextureTiling tiling)
{
    return static_cast<VkImageTiling>(static_cast<uint32_t>(tiling));
}

TextureTiling vkImageTilingToTextureTiling(VkImageTiling tiling)
{
    return static_cast<TextureTiling>(static_cast<uint32_t>(tiling));
}

VkImageLayout textureLayoutToVkImageLayout(TextureLayout layout)
{
    return static_cast<VkImageLayout>(static_cast<uint32_t>(layout));
}

TextureLayout vkImageLayoutToTextureLayout(VkImageLayout layout)
{
    return static_cast<TextureLayout>(static_cast<uint32_t>(layout));
}

ViewType vkImageViewTypeToViewType(VkImageViewType viewType)
{
    return static_cast<ViewType>(static_cast<uint32_t>(viewType));
}

VkImageViewType viewTypeToVkImageViewType(ViewType viewType)
{
    return static_cast<VkImageViewType>(static_cast<uint32_t>(viewType));
}

MemoryUsage vmaMemoryUsageToMemoryUsage(VmaMemoryUsage memoryUsage)
{
    return static_cast<MemoryUsage>(static_cast<uint32_t>(memoryUsage));
}

VmaMemoryUsage memoryUsageToVmaMemoryUsage(MemoryUsage memoryUsage)
{
    return static_cast<VmaMemoryUsage>(static_cast<uint32_t>(memoryUsage));
}

VkDescriptorType resourceBindingTypeToVkDescriptorType(ResourceBindingType type)
{
    return static_cast<VkDescriptorType>(static_cast<uint32_t>(type));
}

ResourceBindingType vkDescriptorTypeToVkDescriptorType(VkDescriptorType type)
{
    return static_cast<ResourceBindingType>(static_cast<uint32_t>(type));
}

VkShaderStageFlagBits shaderStageFlagBitsToVkShaderStageFlagBits(ShaderStageFlagBits stage)
{
    return static_cast<VkShaderStageFlagBits>(static_cast<uint32_t>(stage));
}

ShaderStageFlagBits vkShaderStageFlagBitsToShaderStageFlagBits(VkShaderStageFlagBits stage)
{
    return static_cast<ShaderStageFlagBits>(static_cast<uint32_t>(stage));
}

VkVertexInputRate vertexRateToVkVertexInputRate(VertexRate rate)
{
    return static_cast<VkVertexInputRate>(static_cast<uint32_t>(rate));
}

VertexRate vkVertexInputRateToVertexRate(VkVertexInputRate rate)
{
    return static_cast<VertexRate>(static_cast<uint32_t>(rate));
}

VkPrimitiveTopology primitiveTopologyToVkPrimitiveTopology(PrimitiveTopology topology)
{
    return static_cast<VkPrimitiveTopology>(static_cast<uint32_t>(topology));
}

PrimitiveTopology vkPrimitiveTopologyToPrimitiveTopology(VkPrimitiveTopology topology)
{
    return static_cast<PrimitiveTopology>(static_cast<uint32_t>(topology));
}

VkPolygonMode polygonModeToVkPolygonMode(PolygonMode mode)
{
    return static_cast<VkPolygonMode>(static_cast<uint32_t>(mode));
}

PolygonMode vkPolygonModeToPolygonMode(VkPolygonMode mode)
{
    return static_cast<PolygonMode>(static_cast<uint32_t>(mode));
}

VkFrontFace frontFaceToVkFrontFace(FrontFace face)
{
    return static_cast<VkFrontFace>(static_cast<uint32_t>(face));
}

FrontFace vkFrontFaceToFrontFace(VkFrontFace face)
{
    return static_cast<FrontFace>(static_cast<uint32_t>(face));
}

VkCompareOp compareOperationToVkCompareOp(CompareOperation op)
{
    return static_cast<VkCompareOp>(static_cast<uint32_t>(op));
}

CompareOperation vkCompareOpToCompareOperation(VkCompareOp op)
{
    return static_cast<CompareOperation>(static_cast<uint32_t>(op));
}

VkStencilOp stencilOperationToVkStencilOp(StencilOperation op)
{
    return static_cast<VkStencilOp>(static_cast<uint32_t>(op));
}

StencilOperation vkStencilOpToStencilOperation(StencilOperation op)
{
    return static_cast<StencilOperation>(static_cast<uint32_t>(op));
}

VkBlendFactor blendFactorToVkBlendFactor(BlendFactor factor)
{
    return static_cast<VkBlendFactor>(static_cast<uint32_t>(factor));
}

BlendFactor vkBlendFactorToBlendFactor(VkBlendFactor factor)
{
    return static_cast<BlendFactor>(static_cast<uint32_t>(factor));
}

VkBlendOp blendOperationToVkBlendOp(BlendOperation op)
{
    return static_cast<VkBlendOp>(static_cast<uint32_t>(op));
}

BlendOperation vkBlendOpToBlendOperation(VkBlendOp op)
{
    return static_cast<BlendOperation>(static_cast<uint32_t>(op));
}

VkAttachmentLoadOp attachmentLoadOperationToVkAttachmentLoadOp(AttachmentLoadOperation op)
{
    return static_cast<VkAttachmentLoadOp>(static_cast<uint32_t>(op));
}

AttachmentLoadOperation vkAttachmentLoadOperationToAttachmentLoadOperation(VkAttachmentLoadOp op)
{
    return static_cast<AttachmentLoadOperation>(static_cast<uint32_t>(op));
}

VkAttachmentStoreOp attachmentStoreOperationToVkAttachmentStoreOp(AttachmentStoreOperation op)
{
    return static_cast<VkAttachmentStoreOp>(static_cast<uint32_t>(op));
}

AttachmentStoreOperation vkAttachmentStoreOperationToAttachmentStoreOperation(VkAttachmentStoreOp op)
{
    return static_cast<AttachmentStoreOperation>(static_cast<uint32_t>(op));
}

VkFilter filterModeToVkFilterMode(FilterMode mode)
{
    return static_cast<VkFilter>(static_cast<uint32_t>(mode));
}

VkSamplerMipmapMode mipMapFilterModeToVkSamplerMipmapMode(MipmapFilterMode mode)
{
    return static_cast<VkSamplerMipmapMode>(static_cast<uint32_t>(mode));
}

VkSamplerAddressMode addressModeToVkSamplerAddressMode(AddressMode mode)
{
    return static_cast<VkSamplerAddressMode>(static_cast<uint32_t>(mode));
}

VkIndexType indexTypeToVkIndexType(IndexType type)
{
    return static_cast<VkIndexType>(static_cast<uint32_t>(type));
}

IndexType vkIndexTypeToIndexType(VkIndexType type)
{
    return static_cast<IndexType>(static_cast<uint32_t>(type));
}

VkAccessFlagBits accessFlagsToVkAccessFlagBits(AccessFlags accessFlags)
{
    if (accessFlags.toInt() > VK_ACCESS_FLAG_BITS_MAX_ENUM) {
        SPDLOG_LOGGER_WARN(Logger::logger(), "The requested AccessFlags are not supported without the VK_KHR_Synchronization2 extension.");
    }
    return static_cast<VkAccessFlagBits>(static_cast<uint32_t>(accessFlags.toInt()));
}

#if defined(VK_KHR_synchronization2)
VkAccessFlagBits2KHR accessFlagsToVkAccessFlagBits2(AccessFlags accessFlags)
{
    return static_cast<VkAccessFlagBits2KHR>(accessFlags.toInt());
}
#endif

VkPipelineStageFlagBits pipelineStageFlagsToVkPipelineStageFlagBits(PipelineStageFlags pipelineFlags)
{
    if (pipelineFlags.toInt() > VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM) {
        SPDLOG_LOGGER_WARN(Logger::logger(), "The requested PipelineStageFlags are not supported without the VK_KHR_Synchronization2 extension.");
    }
    return static_cast<VkPipelineStageFlagBits>(static_cast<uint32_t>(pipelineFlags.toInt()));
}

#if defined(VK_KHR_synchronization2)
VkPipelineStageFlagBits2KHR pipelineStageFlagsToVkPipelineStageFlagBits2(PipelineStageFlags pipelineFlags)
{
    return static_cast<VkPipelineStageFlagBits2KHR>(pipelineFlags.toInt());
}
#endif

VkCommandBufferLevel commandBufferLevelToVkCommandBufferLevel(CommandBufferLevel level)
{
    return static_cast<VkCommandBufferLevel>(static_cast<uint32_t>(level));
}

ExternalSemaphoreHandleTypeFlags vkExternalSemaphoreHandleTypeToExternalSemaphoreHandleType(VkExternalSemaphoreHandleTypeFlagBits handleFlags)
{
    return ExternalSemaphoreHandleTypeFlags::fromInt(static_cast<uint32_t>(handleFlags));
}

VkExternalSemaphoreHandleTypeFlagBits externalSemaphoreHandleTypeToVkExternalSemaphoreHandleType(ExternalSemaphoreHandleTypeFlags handleFlags)
{
    return static_cast<VkExternalSemaphoreHandleTypeFlagBits>(static_cast<uint32_t>(handleFlags.toInt()));
}

ExternalMemoryHandleTypeFlags vkExternalMemoryHandleTypeToExternalMemoryHandleType(VkExternalMemoryHandleTypeFlagBits handleFlags)
{
    return ExternalMemoryHandleTypeFlags::fromInt(static_cast<uint32_t>(handleFlags));
}

VkExternalMemoryHandleTypeFlagBits externalMemoryHandleTypeToVkExternalMemoryHandleType(ExternalMemoryHandleTypeFlags handleFlags)
{
    return static_cast<VkExternalMemoryHandleTypeFlagBits>(static_cast<uint32_t>(handleFlags.toInt()));
}

ExternalFenceHandleTypeFlags vkExternalFenceHandleTypeToExternalFenceHandleType(VkExternalFenceHandleTypeFlagBits handleFlags)
{
    return ExternalFenceHandleTypeFlags::fromInt(static_cast<uint32_t>(handleFlags));
}

VkExternalFenceHandleTypeFlagBits externalFenceHandleTypeToVkExternalFenceHandleType(ExternalFenceHandleTypeFlags handleFlags)
{
    return static_cast<VkExternalFenceHandleTypeFlagBits>(static_cast<uint32_t>(handleFlags.toInt()));
}

ResolveModeFlagBits vkResolveModeToResolveMode(VkResolveModeFlagBits resolveFlag)
{
    return static_cast<ResolveModeFlagBits>(static_cast<uint32_t>(resolveFlag));
}

VkResolveModeFlagBits resolveModeToVkResolveMode(ResolveModeFlagBits resolveFlag)
{
    return static_cast<VkResolveModeFlagBits>(static_cast<uint32_t>(resolveFlag));
}

ResolveModeFlags vkResolveModesToResolveModes(VkResolveModeFlags resolveFlags)
{
    return ResolveModeFlags::fromInt(static_cast<uint32_t>(resolveFlags));
}

VkResolveModeFlags resolveModesToVkResolveModes(ResolveModeFlags resolveFlags)
{
    return static_cast<VkResolveModeFlagBits>(static_cast<uint32_t>(resolveFlags.toInt()));
}

VkStencilFaceFlagBits stencilFaceToVkStencilFace(StencilFaceFlags flags)
{
    return static_cast<VkStencilFaceFlagBits>(flags.toInt());
}

VkDynamicState dynamicStateToVkDynamicState(DynamicState state)
{
    return static_cast<VkDynamicState>(state);
}

VkDescriptorBindingFlags resourceBindingFlagsToVkDescriptorBindingFlags(ResourceBindingFlags flags)
{
    return static_cast<VkDescriptorBindingFlags>(flags.toInt());
}

VkBuildAccelerationStructureModeKHR accelerationStructureModeToVkStructureMode(BuildAccelerationStructureMode mode)
{
    return static_cast<VkBuildAccelerationStructureModeKHR>(mode);
}

VkAccelerationStructureTypeKHR accelerationStructureTypeToVkAccelerationStructureType(AccelerationStructureType type)
{
    return static_cast<VkAccelerationStructureTypeKHR>(type);
}

VkGeometryInstanceFlagsKHR geometryInstanceFlagsToVkGeometryInstanceFlags(GeometryInstanceFlags flags)
{
    return static_cast<VkGeometryInstanceFlagsKHR>(flags.toInt());
}

VkRayTracingShaderGroupTypeKHR rayTracingShaderGroupTypeToVkRayTracingShaderGroupType(RayTracingShaderGroupType type)
{
    return static_cast<VkRayTracingShaderGroupTypeKHR>(type);
}

VkBuildAccelerationStructureFlagsKHR accelerationStructureFlagsToVkBuildAccelerationStructureFlags(AccelerationStructureFlags flags)
{
    return static_cast<VkBuildAccelerationStructureFlagsKHR>(flags.toInt());
}

VkDependencyFlags dependencyFlagsToVkDependencyFlags(DependencyFlags flags)
{
    return static_cast<VkDependencyFlags>(flags.toInt());
}

#if defined(VK_EXT_host_image_copy)
VkHostImageCopyFlagsEXT hostImageCopyFlagsToVkHostImageCopyFlags(HostImageCopyFlags flags)
{
    return static_cast<VkHostImageCopyFlagsEXT>(flags.toInt());
}
#endif

VkImageAspectFlags textureAspectFlagsToVkImageAspectFlags(TextureAspectFlags flags)
{
    return static_cast<VkImageAspectFlags>(flags.toInt());
}

VkSamplerYcbcrModelConversion samplerYCbCrModelConvertionToVkSamplerYCbCrModelConversion(SamplerYCbCrModelConversion conversion)
{
    return static_cast<VkSamplerYcbcrModelConversion>(conversion);
}

VkSamplerYcbcrRange samplerYCbCrRangeToVkSamplerYCbCrRange(SamplerYCbCrRange range)
{
    return static_cast<VkSamplerYcbcrRange>(range);
}

VkChromaLocation chromaLocationToVkChromaLocation(ChromaLocation location)
{
    return static_cast<VkChromaLocation>(location);
}

VkComponentSwizzle componentSwizzleToVkComponentSwizzle(ComponentSwizzle swizzle)
{
    return static_cast<VkComponentSwizzle>(swizzle);
}

VkImageCreateFlags textureCreateFlagsToVkImageCreateFlags(TextureCreateFlags flags)
{
    return static_cast<VkImageCreateFlags>(flags.toInt());
}

VkDescriptorPoolCreateFlags bindGroupPoolFlagsToVkDescriptorPoolCreateFlags(BindGroupPoolFlags flags)
{
    return static_cast<VkDescriptorPoolCreateFlags>(flags.toInt());
}

} // namespace KDGpu
