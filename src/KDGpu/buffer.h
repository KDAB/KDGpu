#pragma once

#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>

namespace KDGpu {

struct Device_t;
struct Buffer_t;
struct BufferOptions;
class GraphicsApi;

class KDGPU_EXPORT Buffer
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
    explicit Buffer(GraphicsApi *api, const Handle<Device_t> &device, const BufferOptions &options, const void *initialData);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<Buffer_t> m_buffer;

    void *m_mapped{ nullptr };

    friend class Device;
    friend KDGPU_EXPORT bool operator==(const Buffer &, const Buffer &);
};

KDGPU_EXPORT bool operator==(const Buffer &a, const Buffer &b);
KDGPU_EXPORT bool operator!=(const Buffer &a, const Buffer &b);

} // namespace KDGpu
