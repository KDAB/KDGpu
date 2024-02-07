/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/compositor.h>
#include <KDXr/kdxr_core.h>
#include <KDXr/handle.h>
#include <KDXr/locate_views_options.h>

#include <KDGpu/gpu_core.h>

#include <vector>

namespace KDXr {

class Session;
struct AttachActionSetsOptions;
struct Session_t;

/**
 * @brief ApiSession
 * \ingroup api
 *
 */
struct ApiSession {
    virtual void initialize(Session *frontendSession) = 0;
    virtual std::vector<KDGpu::Format> supportedSwapchainFormats() const = 0;
    virtual FrameState waitForFrame() = 0;
    virtual BeginFrameResult beginFrame() = 0;
    virtual EndFrameResult endFrame(const EndFrameOptions &options) = 0;
    virtual LocateViewsResult locateViews(const LocateViewsOptions &options, ViewConfigurationType viewConfigurationType, ViewState &viewState) = 0;
    virtual AttachActionSetsResult attachActionSets(const AttachActionSetsOptions &options) = 0;
};

} // namespace KDXr
