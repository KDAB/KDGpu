#pragma once

#include <toy_renderer/api/api_adapter.h>

#include <vulkan/vulkan.h>

namespace ToyRenderer {
    
class GraphicsApi;

struct VulkanAdapter : public ApiAdapter {
    explicit VulkanAdapter(VkPhysicalDevice _physicalDevice);

    AdapterProperties queryAdapterProperties() final;
    AdapterFeatures queryAdapterFeatures() final;
    std::vector<AdapterQueueType> queryQueueTypes() final;

    VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };
};

} // namespace ToyRenderer
