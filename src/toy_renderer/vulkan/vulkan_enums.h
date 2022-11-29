#pragma once

#include <toy_renderer/gpu_core.h>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace ToyRenderer {

AdapterDeviceType vkPhysicalDeviceTypeToAdapterDeviceType(VkPhysicalDeviceType deviceType);

Format vkFormatToFormat(VkFormat format);
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

} // namespace ToyRenderer
