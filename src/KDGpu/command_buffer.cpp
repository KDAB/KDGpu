/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "command_buffer.h"
#include <KDGpu/graphics_api.h>
#include <KDGpu/resource_manager.h>
#include <KDGpu/api/api_command_buffer.h>

namespace KDGpu {

CommandBuffer::CommandBuffer() = default;

CommandBuffer::CommandBuffer(GraphicsApi *api,
                             const Handle<Device_t> &device,
                             const Handle<CommandBuffer_t> &commandBuffer)
    : m_api(api)
    , m_device(device)
    , m_commandBuffer(commandBuffer)
{
}

CommandBuffer::CommandBuffer(CommandBuffer &&other)
{
    m_api = other.m_api;
    m_device = other.m_device;
    m_commandBuffer = other.m_commandBuffer;

    other.m_api = nullptr;
    other.m_device = {};
    other.m_commandBuffer = {};
}

CommandBuffer &CommandBuffer::operator=(CommandBuffer &&other)
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteCommandBuffer(handle());

        m_api = other.m_api;
        m_device = other.m_device;
        m_commandBuffer = other.m_commandBuffer;

        other.m_api = nullptr;
        other.m_device = {};
        other.m_commandBuffer = {};
    }
    return *this;
}

CommandBuffer::~CommandBuffer()
{
    if (isValid())
        m_api->resourceManager()->deleteCommandBuffer(handle());
}

bool operator==(const CommandBuffer &a, const CommandBuffer &b)
{
    return a.m_api == b.m_api && a.m_device == b.m_device && a.m_commandBuffer == b.m_commandBuffer;
}

bool operator!=(const CommandBuffer &a, const CommandBuffer &b)
{
    return !(a == b);
}

} // namespace KDGpu
