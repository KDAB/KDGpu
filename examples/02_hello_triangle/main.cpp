#include <toy_renderer/buffer_options.h>
#include <toy_renderer/device.h>
#include <toy_renderer/instance.h>
#include <toy_renderer/formatters.h>
#include <toy_renderer/gpu_core.h>
#include <toy_renderer/swapchain.h>
#include <toy_renderer/swapchain_options.h>
#include <toy_renderer/texture.h>
#include <toy_renderer/texture_options.h>
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

    // Create a window and platform surface from it suitable for use with our chosen graphics API.
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
    SurfaceOptions surfaceOptions = {
        .hWnd = win32Window->handle()
    };
#endif

#if defined(TOY_RENDERER_PLATFORM_LINUX)
    auto xcbWindow = dynamic_cast<LinuxXcbPlatformWindow *>(window.platformWindow());
    SurfaceOptions surfaceOptions = {
        .connection = xcbWindow->connection(),
        .window = xcbWindow->handle()
    };
#endif
    Surface surface = instance.createSurface(surfaceOptions);

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

    // We are now able to query the adapter for swapchain properties and presentation support with the window surface
    const auto swapchainProperties = selectedAdapter.swapchainProperties(surface);
    const bool supportsPresentation = selectedAdapter.supportsPresentation(surface, 0); // Query about the 1st queue type
    spdlog::critical("Queue family 0 supports presentation: {}", supportsPresentation);

    if (!supportsPresentation || !hasGraphicsAndCompute) {
        spdlog::critical("Selected adapter queue family 0 does not meet requirements. Aborting.");
        return -1;
    }

    // Now we can create a device from the selected adapter that we can then use to interact with the GPU.
    auto device = selectedAdapter.createDevice();
    auto queue = device.queues()[0];

    // Create a swapchain of images that we will render to.
    SwapchainOptions swapchainOptions = {
        .surface = surface.handle(),
        .imageExtent = { .width = window.width(), .height = window.height() }
    };
    auto swapchain = device.createSwapchain(swapchainOptions);
    auto swapchainTextures = swapchain.textures();
    const auto swapchainTextureCount = swapchainTextures.size();
    std::vector<TextureView> swapchainViews;
    swapchainViews.reserve(swapchainTextureCount);
    for (uint32_t i = 0; i < swapchainTextureCount; ++i) {
        auto view = swapchainTextures[i].createView({ .format = swapchainOptions.format });
        swapchainViews.push_back(view);
    }

    // Create a depth texture to use for rendering
    auto texture = device.createTexture(
            { .type = TextureType::TextureType2D,
              .format = Format::D24_UNORM_S8_UINT,
              .extent = { window.width(), window.height(), 1 },
              .mipLevels = 1,
              .usage = TextureUsageFlags(TextureUsageFlagBits::DepthStencilAttachmentBit) });

    // Create a buffer to hold triangle vertex data
    BufferOptions bufferOptions = {
        .size = 3 * 2 * 4 * sizeof(float), // 3 vertices * 2 attributes * 4 float components
        .usage = BufferUsageFlags(BufferUsageFlagBits::VertexBufferBit), // TODO: Use a nice Flags template class
        .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
    };
    auto buffer = device.createBuffer(bufferOptions);

    // clang-format off
    std::vector<float> vertexData = {
         1.0f, -1.0f, 0.0f, 1.0f, // position
         1.0f,  0.0f, 0.0f, 1.0f, // color
        -1.0f, -1.0f, 0.0f, 1.0f, // position
         0.0f,  1.0f, 0.0f, 1.0f, // color
         0.0f,  1.0f, 0.0f, 1.0f, // position
         0.0f,  0.0f, 1.0f, 1.0f, // color
    };
    // clang-format on
    auto bufferData = buffer.map();
    std::memcpy(bufferData, vertexData.data(), vertexData.size() * sizeof(float));
    buffer.unmap();

    // TODO: Upload the data to the buffer at creation (needs command recording, barriers etc)
    // auto buffer = device.createBuffer(bufferOptions, vertexData.data());

    // TODO: Create a vertex shader and fragment shader (spir-v only for now)

    // TODO: Create a pipeline layout (array of bind group layouts)

    // TODO: Create a pipeline

    // TODO:    Implement the render loop {
    //              Acquire next swapchain image
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
