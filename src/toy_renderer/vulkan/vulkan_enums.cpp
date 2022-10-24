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

} // namespace ToyRenderer
