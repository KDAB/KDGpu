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
    if (!m_mapped) {
        auto apiBuffer = m_api->resourceManager()->getBuffer(m_buffer);
        m_mapped = apiBuffer->map();
    }
    return m_mapped;
}

void Buffer::unmap()
{
    auto apiBuffer = m_api->resourceManager()->getBuffer(m_buffer);
    apiBuffer->unmap();
    m_mapped = nullptr;
}

} // namespace ToyRenderer
