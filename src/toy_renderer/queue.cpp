#include "queue.h"

#include <toy_renderer/queue.h>

namespace ToyRenderer {

Queue::Queue()
{
}

Queue::Queue(GraphicsApi *api, const QueueDescription &description)
    : m_api(api)
    , m_queue(description.queue)
    , m_flags(description.flags)
    , m_timestampValidBits(description.timestampValidBits)
    , m_minImageTransferGranularity(description.minImageTransferGranularity)
    , m_queueTypeIndex(description.queueTypeIndex)
{
}

Queue::~Queue()
{
}

} // namespace ToyRenderer
