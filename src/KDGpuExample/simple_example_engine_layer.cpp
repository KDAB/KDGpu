/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "simple_example_engine_layer.h"

#include <KDGpuExample/config.h>
#include <KDGpuExample/engine.h>

#include <KDGpu/buffer_options.h>
#include <KDGpu/swapchain_options.h>
#include <KDGpu/texture_options.h>
#include <KDGui/gui_events.h>

// TODO: Move to a better location. This is just to test the OpenXR integration
#if defined(KDGPU_OPENXR)
#include <openxr/openxr.h>
#endif

#include <algorithm>

namespace KDGpuExample {

SimpleExampleEngineLayer::~SimpleExampleEngineLayer()
{
#if defined(KDGPU_OPENXR)
    // Just try calling something simple in OpenXR to see if it works
    uint32_t apiLayerCount = 0;
    std::vector<XrApiLayerProperties> apiLayerProperties;
    if (xrEnumerateApiLayerProperties(0, &apiLayerCount, nullptr) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to enumerate API layers");
        return;
    }
    SPDLOG_LOGGER_CRITICAL(m_logger, "Found {} OpenXR API layers", apiLayerCount);
#endif
}

void SimpleExampleEngineLayer::update()
{
    // Call the base class to delegate any ImGui overlay drawing
    ExampleEngineLayer::update();

    // Release any staging buffers we are done with
    releaseStagingBuffers();

    // Call updateScene() function to update scene state.
    updateScene();

    if (m_swapchainDirty) {
        // We need to recreate swapchain
        recreateSwapChain();
        // Handle any changes that would be needed when a swapchain resize occurs
        resize();
        m_swapchainDirty = false;
    }

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

void SimpleExampleEngineLayer::event(KDFoundation::EventReceiver *target, KDFoundation::Event *ev)
{
    if (ev->type() == KDFoundation::Event::Type::Resize && m_device.isValid())
        m_swapchainDirty = true;

    ExampleEngineLayer::event(target, ev);
}

} // namespace KDGpuExample
