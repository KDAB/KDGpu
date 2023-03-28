#pragma once

#include <kdgpu/graphics_api.h>
#include <kdgpu/kdgpu_export.h>
#include <kdgpu/vulkan/vulkan_resource_manager.h>

#include <memory>

namespace KDGpu {

class KDGPU_EXPORT VulkanGraphicsApi : public GraphicsApi
{
public:
    VulkanGraphicsApi();
    ~VulkanGraphicsApi() final;

private:
    std::unique_ptr<VulkanResourceManager> m_vulkanResourceManager;
};

} // namespace KDGpu
