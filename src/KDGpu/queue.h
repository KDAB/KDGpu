/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/buffer.h>
#include <KDGpu/command_buffer.h>
#include <KDGpu/command_recorder.h>
#include <KDGpu/fence.h>
#include <KDGpu/gpu_core.h>
#include <KDGpu/handle.h>
#include <KDGpu/queue_description.h>
#include <KDGpu/kdgpu_export.h>

#include <vector>

namespace KDGpu {

class GraphicsApi;
class Surface;

struct Adapter_t;
struct Buffer_t;
struct CommandBuffer_t;
struct Device_t;
struct Fence_t;
struct GpuSemaphore_t;
struct Swapchain_t;
struct Texture_t;

struct SubmitOptions {
    std::vector<Handle<CommandBuffer_t>> commandBuffers;
    std::vector<Handle<GpuSemaphore_t>> waitSemaphores;
    std::vector<Handle<GpuSemaphore_t>> signalSemaphores;
    Handle<Fence_t> signalFence;
};

/**
    @ingroup public
    @headerfile queue.h <KDGpu/queue.h>
*/
struct SwapchainPresentInfo {
    Handle<Swapchain_t> swapchain;
    uint32_t imageIndex;
};

/**
    @ingroup public
    @headerfile queue.h <KDGpu/queue.h>
*/
struct PresentOptions {
    std::vector<Handle<GpuSemaphore_t>> waitSemaphores;
    std::vector<SwapchainPresentInfo> swapchainInfos;
};

/**
    @ingroup public
    @headerfile queue.h <KDGpu/queue.h>
*/
struct WaitForBufferUploadOptions {
    Handle<Buffer_t> destinationBuffer;
    const void *data{ nullptr };
    DeviceSize byteSize{ 0 };
    DeviceSize dstOffset{ 0 };
};

/**
    @ingroup public
    @headerfile queue.h <KDGpu/queue.h>
*/
struct BufferUploadOptions {
    Handle<Buffer_t> destinationBuffer;
    PipelineStageFlags dstStages;
    AccessFlags dstMask;
    const void *data{ nullptr };
    DeviceSize byteSize{ 0 };
    DeviceSize dstOffset{ 0 };
};

/**
    @ingroup public
    @headerfile queue.h <KDGpu/queue.h>
*/
struct WaitForTextureUploadOptions {
    Handle<Texture_t> destinationTexture;
    const void *data{ nullptr };
    DeviceSize byteSize{ 0 };
    TextureLayout oldLayout{ TextureLayout::Undefined };
    TextureLayout newLayout{ TextureLayout::Undefined };
    std::vector<BufferTextureCopyRegion> regions;
};

/**
    @ingroup public
    @headerfile queue.h <KDGpu/queue.h>
*/
struct TextureUploadOptions {
    Handle<Texture_t> destinationTexture;
    PipelineStageFlags dstStages;
    AccessFlags dstMask;
    const void *data{ nullptr };
    DeviceSize byteSize{ 0 };
    TextureLayout oldLayout{ TextureLayout::Undefined };
    TextureLayout newLayout{ TextureLayout::Undefined };
    std::vector<BufferTextureCopyRegion> regions;
};

/**
    @ingroup public
    @headerfile queue.h <KDGpu/queue.h>
*/
struct UploadStagingBuffer {
    Fence fence;
    Buffer buffer;
    CommandBuffer commandBuffer;
};

class KDGPU_EXPORT Queue
{
public:
    Queue();
    ~Queue();

    const Handle<Queue_t> &handle() const noexcept { return m_queue; }
    bool isValid() const noexcept { return m_queue.isValid(); }

    operator Handle<Queue_t>() const noexcept { return m_queue; }

    QueueFlags flags() const noexcept { return m_flags; }
    uint32_t timestampValidBits() const noexcept { return m_timestampValidBits; }
    Extent3D minImageTransferGranularity() const noexcept { return m_minImageTransferGranularity; }
    uint32_t queueTypeIndex() const noexcept { return m_queueTypeIndex; }

    void waitUntilIdle();
    void submit(const SubmitOptions &options);

    PresentResult present(const PresentOptions &options);
    std::vector<PresentResult> lastPerSwapchainPresentResults() const;

    void waitForUploadBufferData(const WaitForBufferUploadOptions &options);
    UploadStagingBuffer uploadBufferData(const BufferUploadOptions &options);
    void waitForUploadTextureData(const WaitForTextureUploadOptions &options);
    UploadStagingBuffer uploadTextureData(const TextureUploadOptions &options);

private:
    Queue(GraphicsApi *api, const Handle<Device_t> &device, const QueueDescription &queueDescription);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<Queue_t> m_queue;
    QueueFlags m_flags;
    uint32_t m_timestampValidBits;
    Extent3D m_minImageTransferGranularity;
    uint32_t m_queueTypeIndex;

    friend class Device;
    friend class VulkanGraphicsApi;
};

} // namespace KDGpu
