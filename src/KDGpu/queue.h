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
    std::vector<RequiredHandle<CommandBuffer_t>> commandBuffers;
    std::vector<RequiredHandle<GpuSemaphore_t>> waitSemaphores;
    std::vector<RequiredHandle<GpuSemaphore_t>> signalSemaphores;
    OptionalHandle<Fence_t> signalFence;
};

/*!n    \struct SwapchainPresentInfo
    \brief Specifies a swapchain and image index for presentation
    \ingroup public
    \headerfile queue.h <KDGpu/queue.h>
*/
struct SwapchainPresentInfo {
    RequiredHandle<Swapchain_t> swapchain;
    uint32_t imageIndex;
};

/**
    @ingroup public
    @headerfile queue.h <KDGpu/queue.h>
*/
struct PresentOptions {
    std::vector<RequiredHandle<GpuSemaphore_t>> waitSemaphores;
    std::vector<SwapchainPresentInfo> swapchainInfos;
    std::vector<OptionalHandle<Fence_t>> signalFence; // Per Swapchain
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
    PipelineStageFlags dstStages = PipelineStageFlagBit::AllGraphicsBit;
    const void *data{ nullptr };
    DeviceSize byteSize{ 0 };
    TextureLayout oldLayout{ TextureLayout::Undefined };
    TextureLayout newLayout{ TextureLayout::Undefined };
    std::vector<BufferTextureCopyRegion> regions;
    TextureSubresourceRange range{};
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
    TextureSubresourceRange range{};
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

/*!
    \class Queue
    \brief Represents a GPU command queue for submitting work to the device
    \ingroup public
    \headerfile queue.h <KDGpu/queue.h>

    <b>Vulkan equivalent:</b> [VkQueue](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkQueue.html)

    Queue is the interface for submitting command buffers to the GPU for execution. Every Device has
    one or more queues that support different types of operations (graphics, compute, transfer).

    <b>Key responsibilities:</b>
    - Submit command buffers for GPU execution
    - Present rendered images to swapchains
    - Synchronize CPU and GPU with fences and semaphores
    - Upload data to buffers and textures
    - Wait for GPU operations to complete
    .
    <br/>

    <b>Lifetime:</b> Queue objects are created by Device and remain valid for the lifetime of the device.
    You typically retrieve queues from the device and copy them.

    \note Unlike most other KDGpu resources, Queue instances can be copied around.

    ## Usage

    <b>Basic queue usage:</b>

    \snippet kdgpu_doc_snippets.cpp queue_submit

    <b>Synchronization with fences:</b>

    \snippet kdgpu_doc_snippets.cpp queue_submit_fence

    <b>GPU-to-GPU synchronization with semaphores:</b>

    \snippet kdgpu_doc_snippets.cpp queue_submit_semaphore

    <b>Presenting to swapchains:</b>

    \snippet kdgpu_doc_snippets.cpp queue_present

    <b>Queue capabilities:</b>

    \snippet kdgpu_doc_snippets.cpp queue_wait_idle

    ## Vulkan mapping:
    - Queue::submit()->vkQueueSubmit()
    - Queue::present()->vkQueuePresentKHR()
    - Queue::waitUntilIdle()->vkQueueWaitIdle()
    - Queue::uploadBufferData()->staging buffer + vkCmdCopyBuffer()
    - Queue::uploadTextureData()->staging buffer + vkCmdCopyBufferToImage()

    ## See also:
    \sa SubmitOptions, PresentOptions, Device, CommandRecorder, CommandBuffer, Fence, GpuSemaphore, Swapchain
    \sa \ref kdgpu_api_overview
    \sa \ref kdgpu_vulkan_mapping
*/
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
