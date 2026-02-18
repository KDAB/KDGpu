/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>
#include <KDGpu/graphics_api.h>

namespace KDGpu {

struct CommandBuffer_t;
struct Device_t;

/*!
    \brief CommandBuffer
    \ingroup public
    \headerfile command_buffer.h <KDGpu/command_buffer.h>

    A CommandBuffer is a pre-recorded sequence of GPU commands that can be submitted to a Queue for execution.

    \note CommandBuffer instances cannot be created directly by the user. They are only obtained as the result
    of calling CommandRecorder::finish() after recording commands.

    \section command_recording Recording Commands

    Commands are recorded into a CommandBuffer using specialized command recorders:
    - RenderPassCommandRecorder for graphics/rendering commands (drawing, setting pipelines, bind groups, etc.)
    - ComputePassCommandRecorder for compute shader dispatch commands
    - RayTracingPassCommandRecorder for ray tracing commands

    These recorders are obtained from CommandRecorder methods such as:
    - CommandRecorder::beginRenderPass()
    - CommandRecorder::beginComputePass()
    - CommandRecorder::beginRayTracingPass()

    \section workflow Typical Workflow

    1. Create a CommandRecorder from a Device
    2. Begin a rendering pass with CommandRecorder::beginRenderPass(), beginComputePass(), or beginRayTracingPass()
    3. Record commands using the returned pass command recorder
    4. End the pass with the recorder's end() method
    5. Optionally record more passes or commands
    6. Call CommandRecorder::finish() to obtain the final CommandBuffer
    7. Submit the CommandBuffer to a Queue for execution

    ## See also:
    \sa CommandRecorder, RenderPassCommandRecorder, ComputePassCommandRecorder, RayTracingPassCommandRecorder
*/
class KDGPU_EXPORT CommandBuffer
{
public:
    CommandBuffer();
    ~CommandBuffer();

    CommandBuffer(CommandBuffer &&) noexcept;
    CommandBuffer &operator=(CommandBuffer &&) noexcept;

    CommandBuffer(const CommandBuffer &) = delete;
    CommandBuffer &operator=(const CommandBuffer &) = delete;

    /*!
        \brief Returns the internal handle to the CommandBuffer.
        \return Handle<CommandBuffer_t>
    */
    const Handle<CommandBuffer_t> &handle() const noexcept
    {
        return m_commandBuffer;
    }

    /*!
        \brief Checks if the CommandBuffer is valid and can be used.
        \return true if the CommandBuffer is valid, false otherwise
    */
    bool isValid() const noexcept
    {
        return m_commandBuffer.isValid();
    }

    operator Handle<CommandBuffer_t>() const noexcept
    {
        return m_commandBuffer;
    }

private:
    explicit CommandBuffer(GraphicsApi *api, const Handle<Device_t> &device, const Handle<CommandBuffer_t> &commandBuffer);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<CommandBuffer_t> m_commandBuffer;

    friend class CommandRecorder;
    friend KDGPU_EXPORT bool operator==(const CommandBuffer &, const CommandBuffer &);
};

KDGPU_EXPORT bool operator==(const CommandBuffer &a, const CommandBuffer &b);
KDGPU_EXPORT bool operator!=(const CommandBuffer &a, const CommandBuffer &b);

} // namespace KDGpu
