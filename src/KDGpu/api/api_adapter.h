/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/adapter_features.h>
#include <KDGpu/adapter_properties.h>
#include <KDGpu/adapter_queue_type.h>
#include <KDGpu/adapter_swapchain_properties.h>
#include <KDGpu/gpu_core.h>
#include <KDGpu/handle.h>

#include <vector>

namespace KDGpu {

struct Surface_t;

struct ApiAdapter {
    virtual std::vector<Extension> extensions() const = 0;
    virtual AdapterProperties queryAdapterProperties() = 0;
    virtual AdapterFeatures queryAdapterFeatures() = 0;
    virtual AdapterSwapchainProperties querySwapchainProperties(const Handle<Surface_t> &surfaceHandle) = 0;
    virtual std::vector<AdapterQueueType> queryQueueTypes() = 0;
    virtual bool supportsPresentation(const Handle<Surface_t> surfaceHandle, uint32_t queueTypeIndex) = 0;
    virtual FormatProperties formatProperties(Format format) const = 0;
};

} // namespace KDGpu
