#pragma once

#include <toy_renderer/adapter_queue_type.h>
#include <toy_renderer/device_options.h>
#include <toy_renderer/handle.h>
#include <toy_renderer/queue_description.h>

#include <span>
#include <vector>

namespace ToyRenderer {

class ResourceManager;

struct ApiDevice {
    virtual std::vector<QueueDescription> getQueues(ResourceManager *resourceManager,
                                                    const std::vector<QueueRequest> &queueRequests,
                                                    std::span<AdapterQueueType> queueTypes) = 0;

    virtual void waitUntilIdle() = 0;
};

} // namespace ToyRenderer
