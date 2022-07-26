#include "vulkan_resource_manager.h"

using namespace ToyRenderer;

VulkanResourceManager::VulkanResourceManager()
    : ResourceManager()
{
}

VulkanResourceManager::~VulkanResourceManager()
{
}

Handle<BindGroup> VulkanResourceManager::createBindGroup(BindGroupDescription desc)
{
    // TODO: This is where we will call vkAllocateDescriptorSets
    return {};
}

void VulkanResourceManager::deleteBindGroup(Handle<BindGroup> handle)
{
}
