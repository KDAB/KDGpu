#pragma once

#include <toy_renderer/gpu_core.h>

#include <vulkan/vulkan.h>

namespace ToyRenderer {

AdapterDeviceType vkPhysicalDeviceTypeToAdapterDeviceType(VkPhysicalDeviceType deviceType);

Format vkFormatToFormat(VkFormat format);

ColorSpace vkColorSpaceKHRToColorSpace(VkColorSpaceKHR colorSpace);

PresentMode vkPresentModeKHRToPresentMode(VkPresentModeKHR presentMode);

SurfaceTransformFlagBits vkSurfaceTransformFlagBitsKHRToSurfaceTransformFlagBits(VkSurfaceTransformFlagBitsKHR transform);

} // namespace ToyRenderer
