#include <toy_renderer/instance.h>
#include <toy_renderer/formatters.h>
#include <toy_renderer/gpu_core.h>
#include <toy_renderer/vulkan/vulkan_graphics_api.h>

#include <Serenity/gui/gui_application.h>
#include <Serenity/gui/window.h>
#if defined(TOY_RENDERER_PLATFORM_WIN32)
#include <Serenity/gui/platform/win32/win32_platform_window.h>
#endif
#if defined(TOY_RENDERER_PLATFORM_LINUX)
#include <Serenity/gui/platform/linux/xcb/linux_xcb_platform_window.h>
#endif

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <iostream>
#include <map>
#include <memory>
#include <span>
#include <vector>

using namespace Serenity;
using namespace ToyRenderer;

int main()
{
    GuiApplication app;

    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    // OR
    // std::unique_ptr<GraphicsApi> api = std::make_unique<MetalGraphicsApi>();
    // std::unique_ptr<GraphicsApi> api = std::make_unique<DX12GraphicsApi>();
    // std::unique_ptr<GraphicsApi> api = std::make_unique<WebGpuApi>();

    // Request an instance of the api with whatever layers and extensions we wish to request.
    InstanceOptions instanceOptions = {
        .applicationName = "02_hello_triangle",
        .applicationVersion = SERENITY_MAKE_API_VERSION(0, 1, 0, 0)
    };
    auto instance = api->createInstance(instanceOptions);

    // Enumerate the adapters (physical devices) and select one to use. Here we look for
    // a discrete GPU. In a real app, we could fallback to an integrated one.
    Adapter selectedAdapter;
    auto adapters = instance.adapters();
    for (auto &adapter : adapters) {
        const auto properties = adapter.properties();
        spdlog::critical("Found device: Name: {}, Type: {}", properties.deviceName, properties.deviceType);

        if (properties.deviceType == AdapterDeviceType::DiscreteGpu) {
            selectedAdapter = adapter;
            break;
        }
    }

    if (!selectedAdapter.isValid()) {
        spdlog::critical("Unable to find a discrete GPU. Aborting...");
        return -1;
    }

    // We can easily query the adapter for various features, properties and limits.
    spdlog::critical("maxBoundDescriptorSets = {}", selectedAdapter.properties().limits.maxBoundDescriptorSets);
    spdlog::critical("multiDrawIndirect = {}", selectedAdapter.features().multiDrawIndirect);

    auto queueTypes = selectedAdapter.queueTypes();
    const bool hasGraphicsAndCompute = queueTypes[0].supportsFeature(QueueFlags(QueueFlagBits::GraphicsBit) | QueueFlags(QueueFlagBits::ComputeBit));
    spdlog::critical("Queue family 0 graphics and compute support: {}", hasGraphicsAndCompute);

    // TODO: Now we can create a device from the selected adapter that we can then use to interact with the GPU.
    auto device = selectedAdapter.createDevice();
    auto queue = device.queues()[0];

    // TODO: Create a swapchain for the window
    Window window;
    window.width = 1920;
    window.height = 1080;
    window.visible = true;
    window.visible.valueChanged().connect([&app](const bool &visible) {
        if (visible == false)
            app.quit();
    });

#if defined(TOY_RENDERER_PLATFORM_WIN32)
    auto win32Window = dynamic_cast<Win32PlatformWindow *>(window.platformWindow());
    HWND hWnd = win32Window->handle();
    Surface surface = instance.createSurface(hWnd);
#endif

#if defined(TOY_RENDERER_PLATFORM_LINUX)
    auto xcbWindow = dynamic_cast<LinuxXcbPlatformWindow *>(window.platformWindow());
    xcb_window windowHandle = xcbWindow->handle();
    Surface surface = instance.createSurface(windowHandle);
#endif

    // TODO: Create a buffer to hold triangle vertex data

    // TODO: Create a vertex shader and fragment shader (spir-v only for now)

    // TODO: Create a pipeline layout (array of bind group layouts)

    // TODO: Create a pipeline

    // TODO:    Implement the render loop {
    //              Create a command encoder/recorder
    //              Begin render pass
    //              Bind pipeline
    //              Bind vertex buffer
    //              Bind any resources (bind groups (descriptor sets))
    //              Issue draw command
    //              End render pass
    //              End recording
    //              Submit command buffer to queue
    //              Present and request next frame
    //          }

    return app.exec();
}
