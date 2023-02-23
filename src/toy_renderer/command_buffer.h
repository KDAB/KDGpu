#pragma once

#include <toy_renderer/handle.h>
#include <toy_renderer/toy_renderer_export.h>

namespace ToyRenderer {

struct CommandBuffer_t;

class TOY_RENDERER_EXPORT CommandBuffer
{
public:
    ~CommandBuffer();

    const Handle<CommandBuffer_t> &handle() const noexcept { return m_commandBuffer; }
    bool isValid() const noexcept { return m_commandBuffer.isValid(); }

    operator Handle<CommandBuffer_t>() const noexcept { return m_commandBuffer; }

private:
    explicit CommandBuffer(const Handle<CommandBuffer_t> &commandBuffer);

    Handle<CommandBuffer_t> m_commandBuffer;

    friend class CommandRecorder;
};

} // namespace ToyRenderer
