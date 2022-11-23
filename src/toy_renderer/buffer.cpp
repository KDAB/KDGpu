#include "buffer.h"

namespace ToyRenderer {

Buffer::Buffer(GraphicsApi *api, const Handle<Device_t> &device, const Handle<Buffer_t> &buffer)
    : m_api(api)
    , m_device(device)
    , m_buffer(buffer)
{
}

Buffer::~Buffer()
{
}

} // namespace ToyRenderer
