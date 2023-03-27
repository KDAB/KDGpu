#include "buffer.h"

#include <kdgpu/graphics_api.h>
#include <kdgpu/resource_manager.h>
#include <kdgpu/api/api_buffer.h>
#include <kdgpu/buffer_options.h>

namespace KDGpu {

Buffer::Buffer() = default;

Buffer::Buffer(GraphicsApi *api, const Handle<Device_t> &device, const BufferOptions &options, const void *initialData)
    : m_api(api)
    , m_device(device)
    , m_buffer(m_api->resourceManager()->createBuffer(m_device, options, initialData))
{
}

Buffer::Buffer(Buffer &&other)
{
    m_api = other.m_api;
    m_device = other.m_device;
    m_buffer = other.m_buffer;

    other.m_api = nullptr;
    other.m_device = {};
    other.m_buffer = {};
}

Buffer &Buffer::operator=(Buffer &&other)
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteBuffer(handle());

        m_api = other.m_api;
        m_device = other.m_device;
        m_buffer = other.m_buffer;

        other.m_api = nullptr;
        other.m_device = {};
        other.m_buffer = {};
    }
    return *this;
}

Buffer::~Buffer()
{
    if (isValid())
        m_api->resourceManager()->deleteBuffer(handle());
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

} // namespace KDGpu
