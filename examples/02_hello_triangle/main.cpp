#include "gpu/instance.h"
#include "gpu/serenity_gpu_global.h"
#include "gpu/vulkan/vulkan_resource_manager.h"

#include <iostream>
#include <map>
#include <span>
#include <vector>

int main()
{
    // Somewhere we must decide which underlying graphics API we are using.
    // This could be done via build-time or run-time configuration of Serenity.
    // Here in this prototype we will simply hard-wire the selection.
    // Once this is created, we can access it via ResourceManager::instance().
    Gpu::VulkanResourceManager vk;

    Gpu::InstanceOptions options = {
        .applicationName = "02_hello_triangle",
        .applicationVersion = SERENITY_MAKE_API_VERSION(0, 1, 0, 0)
    };
    Gpu::Instance instance(options);

    Gpu::Adapter adapter = instance.requestAdapter();
    // Device device = adapter.requestDevice();

    return 0;
}
