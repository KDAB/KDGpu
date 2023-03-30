#pragma once

#include <KDGpu/api/api_adapter.h>
#include <KDGpu/kdgpu_export.h>
#include <KDGpu/handle.h>

#include <vulkan/vulkan.h>

namespace KDGpu {

class GraphicsApi;
class VulkanResourceManager;

struct Instance_t;

struct KDGPU_EXPORT VulkanAdapter : public ApiAdapter {
    explicit VulkanAdapter(VkPhysicalDevice _physicalDevice,
                           VulkanResourceManager *_vulkanResourceManager,
                           const Handle<Instance_t> &_instanceHandle);

    std::vector<Extension> extensions() const final;
    AdapterProperties queryAdapterProperties() final;
    AdapterFeatures queryAdapterFeatures() final;
    AdapterSwapchainProperties querySwapchainProperties(const Handle<Surface_t> &surfaceHandle) final;
    std::vector<AdapterQueueType> queryQueueTypes() final;
    bool supportsPresentation(const Handle<Surface_t> surfaceHandle, uint32_t queueTypeIndex) final;
    FormatProperties formatProperties(Format format) const final;

    VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };
    VulkanResourceManager *vulkanResourceManager{ nullptr };
    Handle<Instance_t> instanceHandle;
    std::vector<AdapterQueueType> queueTypes;
};

} // namespace KDGpu
