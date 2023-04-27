/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/gpu_core.h>

namespace KDGpu {

struct AdapterQueueType {
    bool supportsFeature(QueueFlags featureFlags) const noexcept { return (flags & featureFlags) == featureFlags; }

    QueueFlags flags;
    uint32_t availableQueues;
    uint32_t timestampValidBits;
    Extent3D minImageTransferGranularity;
};

} // namespace KDGpu
