#include <toy_renderer/instance.h>
#include <toy_renderer/gpu_core.h>
#include <toy_renderer/vulkan/vulkan_graphics_api.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <iostream>
#include <map>
#include <memory>
#include <span>
#include <vector>

using namespace ToyRenderer;

int main()
{
    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    // OR
    // std::unique_ptr<GraphicsApi> api = std::make_unique<MetalGraphicsApi>();
    // std::unique_ptr<GraphicsApi> api = std::make_unique<DX12GraphicsApi>();

    InstanceOptions instanceOptions = {
        .applicationName = "02_hello_triangle",
        .applicationVersion = SERENITY_MAKE_API_VERSION(0, 1, 0, 0)
    };
    auto instance = api->createInstance(instanceOptions);

    auto adapters = instance.adapters();
    for (auto &adapter : adapters) {
        spdlog::critical("Hello Adapter!");

        // TODO: Get adapter features, properties and limits
    }

    return 0;
}
