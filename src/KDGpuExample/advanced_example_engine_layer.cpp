/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "advanced_example_engine_layer.h"

#include <KDGpuExample/engine.h>

namespace KDGpuExample {

void AdvancedExampleEngineLayer::onAttached()
{
    ExampleEngineLayer::onAttached();

    // Create the present complete and render complete semaphores
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        m_frameFences[i] = m_device.createFence();
}

void AdvancedExampleEngineLayer::onDetached()
{
    // Wait until all commands have completed execution
    m_device.waitUntilIdle();
    m_frameFences = {};

    ExampleEngineLayer::onDetached();
}

void AdvancedExampleEngineLayer::update()
{
    // Obtain swapchain image view
    m_inFlightIndex = engine()->frameNumber() % MAX_FRAMES_IN_FLIGHT;

    // Wait for Fence to be signal (should be done by the queue submission)
    m_frameFences[m_inFlightIndex].wait();

    // Try to acquire image from swapchain
    const auto result = m_swapchain.getNextImageIndex(m_currentSwapchainImageIndex,
                                                      m_presentCompleteSemaphores[m_inFlightIndex]);
    if (result == AcquireImageResult::OutOfDate) {
        // We need to recreate swapchain
        recreateSwapChain();
        // Handle any changes that would be needed when a swapchain resize occurs
        resize();
    }

    // Early return if we failed to retrieve the swapchain image (due to resize or other error)
    // Note: m_presentCompleteSemaphores[m_inFlightIndex] is only valid if image acquisition succeeded
    if (result != AcquireImageResult::Success) {
        return;
    }

    // Reset Fence so that we can submit it again
    m_frameFences[m_inFlightIndex].reset();

    // Call the base class to delegate any ImGui overlay drawing
    ExampleEngineLayer::update();

    // Release any staging buffers we are done with
    releaseStagingBuffers();

    // Call updateScene() function to update scene state.
    updateScene();

    // Call subclass render() function to record and submit drawing commands
    render();

    // Present the swapchain image
    PresentOptions presentOptions = {
        .waitSemaphores = { m_renderCompleteSemaphores[m_inFlightIndex] },
        .swapchainInfos = {
                {
                        .swapchain = m_swapchain,
                        .imageIndex = m_currentSwapchainImageIndex,
                },
        }
    };
    m_queue.present(presentOptions);

    // Waiting for Fences in this functions prevents
    // us preparing more frames than MAX_FRAMES_IN_FLIGHT
}

} // namespace KDGpuExample
