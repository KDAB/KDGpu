#include "example_engine_layer.h"

#include <toy_renderer/swapchain_options.h>

ExampleEngineLayer::ExampleEngineLayer()
    : EngineLayer()
    , m_api(std::make_unique<VulkanGraphicsApi>())
{
}

ExampleEngineLayer::~ExampleEngineLayer()
{
}

void ExampleEngineLayer::onAttached()
{
    m_window = std::make_unique<View>();

    // Request an instance of the api with whatever layers and extensions we wish to request.
    // TODO: Pass these options in to the layer
    InstanceOptions instanceOptions = {
        .applicationName = "03_hello_triangle_simple",
        .applicationVersion = SERENITY_MAKE_API_VERSION(0, 1, 0, 0)
    };
    m_instance = m_api->createInstance(instanceOptions);

    m_surface = m_window->createSurface(m_instance);
    auto defaultDevice = m_instance.createDefaultDevice(m_surface);
    m_device = defaultDevice.device;
    m_queue = m_device.queues()[0];

    // TODO: Move swapchain handling to View?
    SwapchainOptions swapchainOptions = {
        .surface = m_surface.handle(),
        .imageExtent = { .width = m_window->width(), .height = m_window->height() }
    };
    m_swapchain = m_device.createSwapchain(swapchainOptions);
}

void ExampleEngineLayer::onDetached()
{
    // TODO: Properly handle destroying the underlying resources

    m_swapchain = {};
    m_queue = {};
    m_device = {};
    m_surface = {};
    m_instance = {};
    m_window = {};
}

void ExampleEngineLayer::update()
{
}
