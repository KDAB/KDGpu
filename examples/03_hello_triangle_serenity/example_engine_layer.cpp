#include "example_engine_layer.h"

#include <toy_renderer_kdgui/engine.h>

#include <toy_renderer/swapchain_options.h>
#include <toy_renderer/texture_options.h>

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

    // Create a drawable surface
    m_surface = m_window->createSurface(m_instance);

    // Create a device and a queue to use
    auto defaultDevice = m_instance.createDefaultDevice(m_surface);
    m_device = defaultDevice.device;
    m_queue = m_device.queues()[0];

    // TODO: Move swapchain handling to View?
    // Create a swapchain of images that we will render to.
    SwapchainOptions swapchainOptions = {
        .surface = m_surface.handle(),
        .format = m_swapchainFormat,
        .imageExtent = { .width = m_window->width(), .height = m_window->height() }
    };
    m_swapchain = m_device.createSwapchain(swapchainOptions);

    auto swapchainTextures = m_swapchain.textures();
    const auto swapchainTextureCount = swapchainTextures.size();
    m_swapchainViews.reserve(swapchainTextureCount);
    for (uint32_t i = 0; i < swapchainTextureCount; ++i) {
        auto view = swapchainTextures[i].createView({ .format = swapchainOptions.format });
        m_swapchainViews.push_back(view);
    }

    // Create a depth texture to use for depth-correct rendering
    TextureOptions depthTextureOptions = {
        .type = TextureType::TextureType2D,
        .format = Format::D24_UNORM_S8_UINT,
        .extent = { m_window->width(), m_window->height(), 1 },
        .mipLevels = 1,
        .usage = TextureUsageFlags(TextureUsageFlagBits::DepthStencilAttachmentBit),
        .memoryUsage = MemoryUsage::GpuOnly
    };
    m_depthTexture = m_device.createTexture(depthTextureOptions);
    m_depthTextureView = m_depthTexture.createView();

    // Create the present complete and render complete semaphores
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        m_presentCompleteSemaphores[i] = m_device.createGpuSemaphore();
        m_renderCompleteSemaphores[i] = m_device.createGpuSemaphore();
    }

    initializeScene();
}

void ExampleEngineLayer::onDetached()
{
    cleanupScene();

    // TODO: Properly handle destroying the underlying resources
    m_presentCompleteSemaphores = {};
    m_renderCompleteSemaphores = {};
    m_depthTextureView = {};
    m_depthTexture = {};
    m_swapchainViews.clear();
    m_swapchain = {};
    m_queue = {};
    m_device = {};
    m_surface = {};
    m_instance = {};
    m_window = {};
}

void ExampleEngineLayer::update()
{
    // Call updateScene() function to update scene state.
    updateScene();

    // Obtain swapchain image view
    m_inFlightIndex = engine()->frameNumber() % MAX_FRAMES_IN_FLIGHT;
    const auto result = m_swapchain.getNextImageIndex(m_currentSwapchainImageIndex,
                                                      m_presentCompleteSemaphores[m_inFlightIndex].handle());
    if (result != true) {
        // Do we need to recreate the swapchain and dependent resources?
        return;
    }

    // Call subclass render() function to record and submit drawing commands
    render();

    // Present the swapchain image
    // clang-format off
    PresentOptions presentOptions = {
        .waitSemaphores = {{ m_renderCompleteSemaphores[m_inFlightIndex].handle() }},
        .swapchainInfos = {{
            .swapchain = m_swapchain.handle(),
            .imageIndex = m_currentSwapchainImageIndex
        }}
    };
    // clang-format on
    m_queue.present(presentOptions);

    // TODO: Use fences to not block here
    m_device.waitUntilIdle();
}
