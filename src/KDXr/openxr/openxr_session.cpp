/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "openxr_session.h"

#include <KDXr/session.h>
#include <KDXr/openxr/openxr_resource_manager.h>
#include <KDXr/utils/logging.h>

#include <KDGpu/graphics_api.h>

namespace KDXr {

OpenXrSession::OpenXrSession(OpenXrResourceManager *_openxrResourceManager,
                             XrSession _session,
                             const Handle<System_t> _systemHandle,
                             KDGpu::GraphicsApi *_graphicsApi,
                             KDGpu::Handle<KDGpu::Device_t> _device,
                             uint32_t queueIndex) noexcept
    : ApiSession()
    , openxrResourceManager(_openxrResourceManager)
    , session(_session)
    , systemHandle(_systemHandle)
    , graphicsApi(_graphicsApi)
    , deviceHandle(_device)
    , queueIndex(queueIndex)
{
}

void OpenXrSession::initialize(Session *_frontendSession)
{
    frontendSession = _frontendSession;
}

std::vector<KDGpu::Format> OpenXrSession::supportedSwapchainFormats() const
{
    // Query the number of swapchain formats supported by the system
    uint32_t swapchainFormatCount = 0;
    if (xrEnumerateSwapchainFormats(session, 0, &swapchainFormatCount, nullptr) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to enumerate SwapchainFormats.");
        return {};
    }

    // Query the swapchain formats supported by the system
    std::vector<int64_t> xrSwapchainFormats;
    xrSwapchainFormats.resize(swapchainFormatCount);
    if (xrEnumerateSwapchainFormats(session, swapchainFormatCount, &swapchainFormatCount, xrSwapchainFormats.data()) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to enumerate SwapchainFormats.");
        return {};
    }

    // Note: KDGpu formats have the same value as the Vulkan formats. So if we are using the Vulkan backend we can just
    // use the KDGpu formats directly. If we are using the Metal or DX12 backend we need to convert the KDGpu formats to the
    // Metal or DX12 formats.
    if (graphicsApi->api() == KDGpu::GraphicsApi::Api::Vulkan) {
        std::vector<KDGpu::Format> formats;
        formats.reserve(xrSwapchainFormats.size());
        for (const auto &xrSwapchainFormat : xrSwapchainFormats) {
            formats.push_back(static_cast<KDGpu::Format>(xrSwapchainFormat));
        }
        return formats;
    } else {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "OpenXrSession::supportedSwapchainFormats(). Unsupported graphics API.");
        return {};
    }
}

void OpenXrSession::setSessionState(SessionState state)
{
    // Forward on fine-grained state to frontend session
    frontendSession->state = state;
}

} // namespace KDXr
