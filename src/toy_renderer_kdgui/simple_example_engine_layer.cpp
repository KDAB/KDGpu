#include "simple_example_engine_layer.h"

#include <toy_renderer_kdgui/engine.h>

#include <toy_renderer/buffer_options.h>
#include <toy_renderer/swapchain_options.h>
#include <toy_renderer/texture_options.h>

#include <algorithm>

namespace ToyRendererKDGui {

SimpleExampleEngineLayer::SimpleExampleEngineLayer()
    : ExampleEngineLayer()
{
}

SimpleExampleEngineLayer::SimpleExampleEngineLayer(const SampleCountFlagBits samples)
    : ExampleEngineLayer(samples)
{
}

SimpleExampleEngineLayer::~SimpleExampleEngineLayer()
{
}

void SimpleExampleEngineLayer::onAttached()
{
    ExampleEngineLayer::onAttached();
    initializeScene();
}

void SimpleExampleEngineLayer::update()
{
    // Release any staging buffers we are done with
    releaseStagingBuffers();

    // Call updateScene() function to update scene state.
    updateScene();

    // Obtain swapchain image view
    m_inFlightIndex = engine()->frameNumber() % MAX_FRAMES_IN_FLIGHT;
    const auto result = m_swapchain.getNextImageIndex(m_currentSwapchainImageIndex,
                                                      m_presentCompleteSemaphores[m_inFlightIndex]);
    if (result == AcquireImageResult::OutOfDate) {
        // We need to recreate swapchain
        recreateSwapChain();
        // Handle any changes that would be needed when a swapchain resize occurs
        resize();
        // Early return as we need to retry to retrieve the image index
        return;
    } else if (result != AcquireImageResult::Success) {
        // Something went wrong and we can't recover from it
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

    // Just wait until the GPU is done with all work
    m_device.waitUntilIdle();
}

} // namespace ToyRendererKDGui
