#pragma once

#include <toy_renderer/handle.h>
#include <toy_renderer/toy_renderer_export.h>

namespace ToyRenderer {

struct Device_t;
struct Buffer_t;

class GraphicsApi;

class TOY_RENDERER_EXPORT Buffer
{
public:
    ~Buffer();
    Buffer();

    Buffer(Buffer &&);
    Buffer &operator=(Buffer &&);

    Buffer(const Buffer &) = delete;
    Buffer &operator=(const Buffer &) = delete;

    const Handle<Buffer_t> &handle() const noexcept { return m_buffer; }
    bool isValid() const noexcept { return m_buffer.isValid(); }

    operator Handle<Buffer_t>() const noexcept { return m_buffer; }

    void *map();
    void unmap();

private:
    explicit Buffer(GraphicsApi *api, const Handle<Device_t> &device, const Handle<Buffer_t> &buffer);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<Buffer_t> m_buffer;

    void *m_mapped{ nullptr };

    friend class Device;
    friend TOY_RENDERER_EXPORT bool operator==(const Buffer &, const Buffer &);
};

TOY_RENDERER_EXPORT bool operator==(const Buffer &a, const Buffer &b);
TOY_RENDERER_EXPORT bool operator!=(const Buffer &a, const Buffer &b);

} // namespace ToyRenderer
