#include "queue.h"

namespace ToyRenderer {

Queue::Queue()
{
}

Queue::Queue(const QueueDescription &description)
    : m_queue(description.queue)
    , m_flags(description.flags)
    , m_timestampValidBits(description.timestampValidBits)
    , m_minImageTransferGranularity(description.minImageTransferGranularity)
{
}

Queue::~Queue()
{
}

} // namespace ToyRenderer
