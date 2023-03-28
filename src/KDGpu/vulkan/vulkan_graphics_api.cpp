#include "vulkan_graphics_api.h"

#include <KDGpu/vulkan/vulkan_enums.h>

namespace KDGpu {

VulkanGraphicsApi::VulkanGraphicsApi()
    : GraphicsApi()
    , m_vulkanResourceManager{ std::make_unique<VulkanResourceManager>() }
{
    m_resourceManager = m_vulkanResourceManager.get();
}

VulkanGraphicsApi::~VulkanGraphicsApi()
{
}

} // namespace KDGpu
