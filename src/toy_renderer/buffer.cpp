#include "buffer.h"

#include <toy_renderer/graphics_api.h>
#include <toy_renderer/resource_manager.h>
#include <toy_renderer/api/api_buffer.h>

namespace ToyRenderer {

Buffer::Buffer()
{
}

Buffer::Buffer(GraphicsApi *api, const Handle<Device_t> &device, const Handle<Buffer_t> &buffer)
    : m_api(api)
    , m_device(device)
    , m_buffer(buffer)
{
}

Buffer::~Buffer()
{
}

void *Buffer::map()
{
    if (!m_mapped && isValid()) {
        auto apiBuffer = m_api->resourceManager()->getBuffer(m_buffer);
        m_mapped = apiBuffer->map();
    }
    return m_mapped;
}

void Buffer::unmap()
{
    if (!m_mapped)
        return;
    auto apiBuffer = m_api->resourceManager()->getBuffer(m_buffer);
    apiBuffer->unmap();
    m_mapped = nullptr;
}

bool operator==(const Buffer &a, const Buffer &b)
{
    return a.m_api == b.m_api && a.m_device == b.m_device && a.m_buffer == b.m_buffer && a.m_mapped == b.m_mapped;
}

bool operator!=(const Buffer &a, const Buffer &b)
{
    return !(a == b);
}

} // namespace ToyRenderer
