/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/gpu_core.h>
#include <KDGpu/handle.h>

namespace KDGpu {

struct Queue_t;

struct QueueDescription {
    const Handle<Queue_t> queue;
    QueueFlags flags;
    uint32_t timestampValidBits;
    Extent3D minImageTransferGranularity;
    uint32_t queueTypeIndex;
};

} // namespace KDGpu
