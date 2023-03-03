#pragma once

#include <toy_renderer/handle.h>
#include <toy_renderer/toy_renderer_export.h>

namespace ToyRenderer {

struct CommandBuffer_t;
struct Device_t;
class GraphicsApi;

class TOY_RENDERER_EXPORT CommandBuffer
{
public:
    CommandBuffer();
    ~CommandBuffer();

    CommandBuffer(CommandBuffer &&);
    CommandBuffer &operator=(CommandBuffer &&);

    CommandBuffer(const CommandBuffer &) = delete;
    CommandBuffer &operator=(const CommandBuffer &) = delete;

    const Handle<CommandBuffer_t> &handle() const noexcept { return m_commandBuffer; }
    bool isValid() const noexcept { return m_commandBuffer.isValid(); }

    operator Handle<CommandBuffer_t>() const noexcept { return m_commandBuffer; }

private:
    explicit CommandBuffer(GraphicsApi *api, const Handle<Device_t> &device, const Handle<CommandBuffer_t> &commandBuffer);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<CommandBuffer_t> m_commandBuffer;

    friend class CommandRecorder;
    friend TOY_RENDERER_EXPORT bool operator==(const CommandBuffer &, const CommandBuffer &);
};

TOY_RENDERER_EXPORT bool operator==(const CommandBuffer &a, const CommandBuffer &b);
TOY_RENDERER_EXPORT bool operator!=(const CommandBuffer &a, const CommandBuffer &b);

} // namespace ToyRenderer
