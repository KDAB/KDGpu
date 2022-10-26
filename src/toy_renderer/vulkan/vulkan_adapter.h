#pragma once

#include <toy_renderer/api/api_adapter.h>

#include <vulkan/vulkan.h>

namespace ToyRenderer {

struct VulkanAdapter : public ApiAdapter {
    explicit VulkanAdapter(VkPhysicalDevice _physicalDevice);

    AdapterProperties queryAdapterProperties() final;
    AdapterFeatures queryAdapterFeatures() final;

    // TODO: Handle<Device_t>
    void createDevice(const DeviceOptions &options) final;

    VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };
};

} // namespace ToyRenderer
