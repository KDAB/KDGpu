/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>
#include <KDGpu/gpu_core.h>
#include <KDGpu/graphics_api.h>

namespace KDGpu {

struct Device_t;
struct Buffer_t;
struct BufferOptions;

/*!
    \class Buffer
    \brief Represents a GPU memory buffer for storing vertex, index, uniform data, etc.
    \ingroup public
    \headerfile buffer.h <KDGpu/buffer.h>

    <b>Vulkan equivalent:</b> [VkBuffer](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkBuffer.html)

    Buffer represents a linear allocation of GPU memory that can store vertices, indices, uniform data,
    storage buffers, or any other structured data needed by shaders.

    <b>Key features:</b>
    - CPU-visible mapping for reading/writing buffer contents
    - Various usage flags (vertex, index, uniform, storage, transfer)
    - Memory placement control (CPU, GPU, or CPU->GPU)
    - Buffer device addresses for bindless rendering
    .
    <br/>

    <b>Lifetime:</b> Buffers are created by Device and must remain valid while referenced by GPU commands.
    They use RAII and clean up automatically when destroyed.

    ## Usage

    <b>Creating a vertex buffer:</b>

    \snippet kdgpu_doc_snippets.cpp buffer_vertex_creation

    <b>Creating an index buffer :</b>

    \snippet kdgpu_doc_snippets.cpp buffer_index_creation

    <b>Creating a uniform buffer(updated frequently) :</b>

    \snippet kdgpu_doc_snippets.cpp buffer_uniform_creation

    <b>Creating a storage buffer(shader read / write):</b>

    \snippet kdgpu_doc_snippets.cpp buffer_storage_creation

    <b>Memory usage patterns:</b>

    \snippet kdgpu_doc_snippets.cpp buffer_memory_usage_patterns

    <b>Mapping and unmapping:</b>

    \snippet kdgpu_doc_snippets.cpp buffer_mapping_unmapping

    <b>Buffer device addresses (for bindless):</b>

    \snippet kdgpu_doc_snippets.cpp buffer_device_address

    ## Vulkan mapping:
    - Buffer creation->vkCreateBuffer() + vkAllocateMemory() + vkBindBufferMemory()
    - Buffer::map()->vkMapMemory() - Buffer::unmap()->vkUnmapMemory()
    - Buffer::flush()->vkFlushMappedMemoryRanges()
    - Buffer::invalidate()->vkInvalidateMappedMemoryRanges()
    - Buffer::bufferDeviceAddress()->vkGetBufferDeviceAddress()

    ## See also:
    \sa Device, BufferOptions, Queue, BufferOptions, BindGroup
    \sa \ref kdgpu_api_overview
    \sa \ref kdgpu_vulkan_mapping
*/
class KDGPU_EXPORT Buffer
{
public:
    ~Buffer();
    Buffer();

    Buffer(Buffer &&) noexcept;
    Buffer &operator=(Buffer &&) noexcept;

    Buffer(const Buffer &) = delete;
    Buffer &operator=(const Buffer &) = delete;

    const Handle<Buffer_t> &handle() const noexcept { return m_buffer; }
    bool isValid() const noexcept { return m_buffer.isValid(); }

    operator Handle<Buffer_t>() const noexcept { return m_buffer; }

    void *map();
    void unmap();

    void invalidate();
    void flush();

    MemoryHandle externalMemoryHandle() const;
    BufferDeviceAddress bufferDeviceAddress() const;

private:
    explicit Buffer(GraphicsApi *api, const Handle<Device_t> &device, const BufferOptions &options, const void *initialData);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<Buffer_t> m_buffer;

    void *m_mapped{ nullptr };

    friend class Device;
    friend class Queue;
    friend KDGPU_EXPORT bool operator==(const Buffer &, const Buffer &);
};

KDGPU_EXPORT bool operator==(const Buffer &a, const Buffer &b);
KDGPU_EXPORT bool operator!=(const Buffer &a, const Buffer &b);

} // namespace KDGpu
