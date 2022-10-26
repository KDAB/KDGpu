#pragma once

#include <toy_renderer/graphics_api.h>
#include <toy_renderer/toy_renderer_export.h>
#include <toy_renderer/vulkan/vulkan_resource_manager.h>

#include <memory>

namespace ToyRenderer {

class TOY_RENDERER_EXPORT VulkanGraphicsApi : public GraphicsApi
{
public:
    VulkanGraphicsApi();
    ~VulkanGraphicsApi() final;

private:
    std::unique_ptr<VulkanResourceManager> m_vulkanResourceManager;
};

} // namespace ToyRenderer
