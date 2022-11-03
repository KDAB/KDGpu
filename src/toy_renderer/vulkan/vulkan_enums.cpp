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

ColorSpace vkColorSpaceKHRToColorSpace(VkColorSpaceKHR colorSpace)
{
    return static_cast<ColorSpace>(static_cast<uint32_t>(colorSpace));
}

PresentMode vkPresentModeKHRToPresentMode(VkPresentModeKHR presentMode)
{
    return static_cast<PresentMode>(static_cast<uint32_t>(presentMode));
}

SurfaceTransformFlagBits vkSurfaceTransformFlagBitsKHRToSurfaceTransformFlagBits(VkSurfaceTransformFlagBitsKHR transform)
{
    return static_cast<SurfaceTransformFlagBits>(static_cast<uint32_t>(transform));
}

} // namespace ToyRenderer
