#pragma once

#include <toy_renderer/gpu_core.h>
#include <toy_renderer/handle.h>
#include <toy_renderer/queue_description.h>

#include <toy_renderer/toy_renderer_export.h>

namespace ToyRenderer {

class TOY_RENDERER_EXPORT Queue
{
public:
    Queue();
    ~Queue();

    bool isValid() const noexcept { return m_queue.isValid(); }

    const Handle<Queue_t> &handle() const noexcept { return m_queue; }
    QueueFlags flags() const noexcept { return m_flags; }
    uint32_t timestampValidBits() const noexcept { return m_timestampValidBits; }
    Extent3D minImageTransferGranularity() const noexcept { return m_minImageTransferGranularity; }

private:
    Queue(const QueueDescription &queueDescription);

    Handle<Queue_t> m_queue;
    QueueFlags m_flags;
    uint32_t m_timestampValidBits;
    Extent3D m_minImageTransferGranularity;

    friend class Device;
};

} // namespace ToyRenderer
