#pragma once

#include <toy_renderer/api/api_adapter.h>

#include <vulkan/vulkan.h>

namespace ToyRenderer {

class GraphicsApi;
class VulkanResourceManager;

struct VulkanAdapter : public ApiAdapter {
    explicit VulkanAdapter(VkPhysicalDevice _physicalDevice, VulkanResourceManager *_vulkanResourceManager);

    AdapterProperties queryAdapterProperties() final;
    AdapterFeatures queryAdapterFeatures() final;
    AdapterSwapchainProperties querySwapchainProperties(const Handle<Surface_t> &surfaceHandle) final;
    std::vector<AdapterQueueType> queryQueueTypes() final;
    bool supportsPresentation(const Handle<Surface_t> surfaceHandle, uint32_t queueTypeIndex) final;

    VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };
    VulkanResourceManager *vulkanResourceManager{ nullptr };
};

} // namespace ToyRenderer
