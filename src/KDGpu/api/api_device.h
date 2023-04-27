/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/adapter_queue_type.h>
#include <KDGpu/device_options.h>
#include <KDGpu/handle.h>
#include <KDGpu/queue_description.h>

#include <span>
#include <vector>

namespace KDGpu {

class ResourceManager;

struct ApiDevice {
    virtual std::vector<QueueDescription> getQueues(ResourceManager *resourceManager,
                                                    const std::vector<QueueRequest> &queueRequests,
                                                    std::span<AdapterQueueType> queueTypes) = 0;

    virtual void waitUntilIdle() = 0;
};

} // namespace KDGpu
