#include "vulkan_graphics_api.h"

#include <toy_renderer/vulkan/vulkan_enums.h>

namespace ToyRenderer {

VulkanGraphicsApi::VulkanGraphicsApi()
    : GraphicsApi()
    , m_vulkanResourceManager{ std::make_unique<VulkanResourceManager>() }
{
    m_resourceManager = m_vulkanResourceManager.get();
}

VulkanGraphicsApi::~VulkanGraphicsApi()
{
}

} // namespace ToyRenderer
