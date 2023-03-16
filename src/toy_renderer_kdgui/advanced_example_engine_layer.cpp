#include "advanced_example_engine_layer.h"

#include <KDGui/gui_application.h>
#include <toy_renderer_kdgui/engine.h>

#include <toy_renderer/swapchain_options.h>
#include <toy_renderer/texture_options.h>

AdvancedExampleEngineLayer::AdvancedExampleEngineLayer()
    : EngineLayer()
    , m_api(std::make_unique<VulkanGraphicsApi>())
{
}

AdvancedExampleEngineLayer::~AdvancedExampleEngineLayer()
{
}

void AdvancedExampleEngineLayer::onAttached()
{
    m_window = std::make_unique<View>();

    // Request an instance of the api with whatever layers and extensions we wish to request.
    // TODO: Pass these options in to the layer
    InstanceOptions instanceOptions = {
        .applicationName = KDGui::GuiApplication::instance()->objectName(),
        .applicationVersion = SERENITY_MAKE_API_VERSION(0, 1, 0, 0)
    };
    m_instance = m_api->createInstance(instanceOptions);

    // Create a drawable surface
    m_surface = m_window->createSurface(m_instance);

    // Create a device and a queue to use
    auto defaultDevice = m_instance.createDefaultDevice(m_surface);
    m_device = std::move(defaultDevice.device);
    m_queue = m_device.queues()[0];

    // TODO: Move swapchain handling to View?
    // Create a swapchain of images that we will render to.
    SwapchainOptions swapchainOptions = {
        .surface = m_surface,
        .format = m_swapchainFormat,
        .imageExtent = { .width = m_window->width(), .height = m_window->height() }
    };
    m_swapchain = m_device.createSwapchain(swapchainOptions);

    const auto &swapchainTextures = m_swapchain.textures();
    const auto swapchainTextureCount = swapchainTextures.size();
    m_swapchainViews.reserve(swapchainTextureCount);
    for (uint32_t i = 0; i < swapchainTextureCount; ++i) {
        auto view = swapchainTextures[i].createView({ .format = swapchainOptions.format });
        m_swapchainViews.push_back(std::move(view));
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
        m_frameFences[i] = m_device.createFence();
    }

    initializeScene();
}

void AdvancedExampleEngineLayer::onDetached()
{
    // Wait until all commands have completed execution
    m_device.waitUntilIdle();

    cleanupScene();

    m_presentCompleteSemaphores = {};
    m_renderCompleteSemaphores = {};
    m_frameFences = {};
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

void AdvancedExampleEngineLayer::update()
{
    // Obtain swapchain image view
    m_inFlightIndex = engine()->frameNumber() % MAX_FRAMES_IN_FLIGHT;

    // Wait for Fence to be signal (should be done by the queue submission)
    m_frameFences[m_inFlightIndex].wait();

    // Reset Fence so that we can submit it again
    m_frameFences[m_inFlightIndex].reset();

    // Call updateScene() function to update scene state.
    updateScene();

    const auto result = m_swapchain.getNextImageIndex(m_currentSwapchainImageIndex,
                                                      m_presentCompleteSemaphores[m_inFlightIndex]);
    if (result != AcquireImageResult::Success) {
        // Do we need to recreate the swapchain and dependent resources?
        return;
    }

    // Call subclass render() function to record and submit drawing commands
    render();

    // Present the swapchain image
    // clang-format off
    PresentOptions presentOptions = {
        .waitSemaphores = { m_renderCompleteSemaphores[m_inFlightIndex] },
        .swapchainInfos = {{
            .swapchain = m_swapchain,
            .imageIndex = m_currentSwapchainImageIndex
        }}
    };
    // clang-format on
    m_queue.present(presentOptions);

    // Waiting for Fences at the beginning of this functions prevents
    // us preparing more frames than MAX_FRAMES_IN_FLIGHT
}
