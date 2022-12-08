#include "vulkan_enums.h"

#include <spdlog/spdlog.h>

namespace ToyRenderer {

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
        spdlog::critical("Unknown adapter type");
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

} // namespace ToyRenderer
