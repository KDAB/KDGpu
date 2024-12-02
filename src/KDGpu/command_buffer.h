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

/**
 * @brief CommandBuffer
 * @ingroup public
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

    const Handle<CommandBuffer_t> &handle() const noexcept { return m_commandBuffer; }
    bool isValid() const noexcept { return m_commandBuffer.isValid(); }

    operator Handle<CommandBuffer_t>() const noexcept { return m_commandBuffer; }

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
