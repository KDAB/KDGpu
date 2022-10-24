#pragma once

#include <toy_renderer/gpu_core.h>

#include <vulkan/vulkan.h>

namespace ToyRenderer {

AdapterDeviceType vkPhysicalDeviceTypeToAdapterDeviceType(VkPhysicalDeviceType deviceType);

}
