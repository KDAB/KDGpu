/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/adapter_features.h>
#include <KDGpu/adapter_group.h>
#include <KDGpu/gpu_core.h>

#include <stdint.h>
#include <string>
#include <vector>

namespace KDGpu {

struct QueueRequest {
    uint32_t queueTypeIndex;
    uint32_t count;
    std::vector<float> priorities;
};

struct DeviceOptions {
    std::string_view label;
    // Version we want the device to use, can be less than the apiVersion requested for the instance
    uint32_t apiVersion{ KDGPU_MAKE_API_VERSION(0, 1, 2, 0) };
    std::vector<std::string> layers;
    std::vector<std::string> extensions;
    std::vector<QueueRequest> queues;
    AdapterFeatures requestedFeatures;
    AdapterGroup adapterGroup;
};

} // namespace KDGpu
