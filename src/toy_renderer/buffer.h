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
    Buffer();
    ~Buffer();

    const Handle<Buffer_t> &handle() const noexcept { return m_buffer; }
    bool isValid() const noexcept { return m_buffer.isValid(); }

    void *map();
    void unmap();

private:
    explicit Buffer(GraphicsApi *api, const Handle<Device_t> &device, const Handle<Buffer_t> &buffer);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<Buffer_t> m_buffer;

    void *m_mapped{ nullptr };

    friend class Device;
};

} // namespace ToyRenderer
