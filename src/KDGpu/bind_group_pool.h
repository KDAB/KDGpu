/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/kdgpu_export.h>
#include <KDGpu/graphics_api.h>
#include <KDGpu/handle.h>

namespace KDGpu {

struct BindGroupPool_t;
struct BindGroupPoolOptions;
struct Device_t;

/*!
    \class BindGroupPool
    \brief Memory pool for allocating bind groups (descriptor sets)
    \ingroup public
    \headerfile bind_group_pool.h <KDGpu/bind_group_pool.h>

    <b>Vulkan equivalent:</b> [VkDescriptorPool](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorPool.html)

    BindGroupPool (known as descriptor pool in Vulkan) is a memory pool from which bind groups (descriptor sets)
    are allocated. It pre-allocates GPU memory for a certain number of descriptors of various types, providing
    efficient allocation and deallocation of bind groups.

    <b>Key features:</b>
    - Pre-allocated memory for descriptor sets
    - Configurable resource limits (uniform buffers, samplers, storage buffers, etc.)
    - Efficient allocation and reset operations
    - Track allocated bind group count
    - Optional: KDGpu can manage a default pool for you

    <b>Lifetime:</b> Pools are created by Device and typically live for the entire rendering session.
    All bind groups allocated from a pool become invalid when the pool is destroyed or reset.

    <b>Basic pool creation:</b>

    \snippet kdgpu_doc_snippets.cpp bindgrouppool_creation

    <b>Using pool with bind groups:</b>

    \snippet kdgpu_doc_snippets.cpp bindgrouppool_allocate

    <b>Resetting pool:</b>

    \snippet kdgpu_doc_snippets.cpp bindgrouppool_reset

    <b>Per-frame pooling strategy:</b>

    \snippet kdgpu_doc_snippets.cpp bindgrouppool_per_frame

    <b>Pool capacity limits:</b>

    \snippet kdgpu_doc_snippets.cpp bindgrouppool_limits
    - BindGroupPool creation -> vkCreateDescriptorPool()
    - BindGroupPool::reset() -> vkResetDescriptorPool()
    - Bind group allocation from pool -> vkAllocateDescriptorSets()

    \sa BindGroup, BindGroupLayout, Device
    \sa \ref kdgpu_api_overview
    \sa \ref kdgpu_vulkan_mapping
*/
class KDGPU_EXPORT BindGroupPool
{
public:
    BindGroupPool();
    ~BindGroupPool();

    BindGroupPool(BindGroupPool &&) noexcept;
    BindGroupPool &operator=(BindGroupPool &&) noexcept;

    BindGroupPool(const BindGroupPool &) = delete;
    BindGroupPool &operator=(const BindGroupPool &) = delete;

    const Handle<BindGroupPool_t> &handle() const noexcept { return m_bindGroupPool; }
    bool isValid() const noexcept { return m_bindGroupPool.isValid(); }

    operator Handle<BindGroupPool_t>() const noexcept { return m_bindGroupPool; }

    void reset();
    uint16_t allocatedBindGroupCount() const;
    uint16_t maxBindGroupCount() const;

private:
    explicit BindGroupPool(GraphicsApi *api, const Handle<Device_t> &device, const BindGroupPoolOptions &options);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<BindGroupPool_t> m_bindGroupPool;

    friend class Device;
    friend KDGPU_EXPORT bool operator==(const BindGroupPool &, const BindGroupPool &);
};

KDGPU_EXPORT bool operator==(const BindGroupPool &a, const BindGroupPool &b);
KDGPU_EXPORT bool operator!=(const BindGroupPool &a, const BindGroupPool &b);

} // namespace KDGpu
