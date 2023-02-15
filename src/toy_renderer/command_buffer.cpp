#include "command_buffer.h"

namespace ToyRenderer {

CommandBuffer::CommandBuffer(const Handle<CommandBuffer_t> &commandBuffer)
    : m_commandBuffer(commandBuffer)
{
}

CommandBuffer::~CommandBuffer()
{
}

} // namespace ToyRenderer
