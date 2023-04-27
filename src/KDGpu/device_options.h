/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/adapter_features.h>

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
    std::vector<std::string> layers;
    std::vector<std::string> extensions;
    std::vector<QueueRequest> queues;
    AdapterFeatures requestedFeatures;
};

} // namespace KDGpu
