/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/command_buffer.h>
#include <KDGpu/handle.h>
#include <KDGpu/compute_pass_command_recorder.h>
#include <KDGpu/render_pass_command_recorder.h>
#include <KDGpu/raytracing_pass_command_recorder.h>
#include <KDGpu/timestamp_query_recorder.h>
#include <KDGpu/timestamp_query_recorder_options.h>
#include <KDGpu/render_pass_command_recorder_options.h>
#include <KDGpu/kdgpu_export.h>
#include <KDGpu/memory_barrier.h>
#include <KDGpu/acceleration_structure_options.h>

namespace KDGpu {

class VulkanGraphicsApi;

struct CommandRecorder_t;
struct Device_t;
struct Queue_t;

struct CommandRecorderOptions {
    Handle<Queue_t> queue; // The queue on which you wish to submit the recorded commands. If not set, defaults to first queue of the device
    CommandBufferLevel level{ CommandBufferLevel::Primary };
};

struct BufferCopy {
    Handle<Buffer_t> src;
    DeviceSize srcOffset{ 0 };
    Handle<Buffer_t> dst;
    DeviceSize dstOffset{ 0 };
    DeviceSize byteSize{ 0 };
};

struct BufferTextureCopyRegion {
    DeviceSize bufferOffset{ 0 };
    uint32_t bufferRowLength{ 0 };
    uint32_t bufferTextureHeight{ 0 };
    TextureSubresourceLayers textureSubResource{};
    Offset3D textureOffset{};
    Extent3D textureExtent{};
};

struct BufferToTextureCopy {
    Handle<Buffer_t> srcBuffer;
    Handle<Texture_t> dstTexture;
    TextureLayout dstTextureLayout;
    std::vector<BufferTextureCopyRegion> regions;
};

struct TextureToBufferCopy {
    Handle<Texture_t> srcTexture;
    TextureLayout srcTextureLayout;
    Handle<Buffer_t> dstBuffer;
    std::vector<BufferTextureCopyRegion> regions;
};

struct TextureCopyRegion {
    TextureSubresourceLayers srcSubresource{ .aspectMask = TextureAspectFlagBits::ColorBit };
    Offset3D srcOffset{};
    TextureSubresourceLayers dstSubresource{ .aspectMask = TextureAspectFlagBits::ColorBit };
    Offset3D dstOffset{};
    Extent3D extent;
};

struct TextureToTextureCopy {
    Handle<Texture_t> srcTexture;
    TextureLayout srcLayout;
    Handle<Texture_t> dstTexture;
    TextureLayout dstLayout;
    std::vector<TextureCopyRegion> regions;
};

struct TextureBlitRegion {
    TextureSubresourceLayers srcSubresource{ .aspectMask = TextureAspectFlagBits::ColorBit };
    Offset3D srcOffset{};
    Extent3D srcExtent{};
    TextureSubresourceLayers dstSubresource{ .aspectMask = TextureAspectFlagBits::ColorBit };
    Offset3D dstOffset{};
    Extent3D dstExtent{};
};

struct TextureBlitOptions {
    Handle<Texture_t> srcTexture;
    TextureLayout srcLayout;
    Handle<Texture_t> dstTexture;
    TextureLayout dstLayout;
    std::vector<TextureBlitRegion> regions;
    FilterMode scalingFilter;
};

using TextureResolveRegion = TextureCopyRegion;

struct TextureResolveOptions {
    Handle<Texture_t> srcTexture;
    TextureLayout srcLayout;
    Handle<Texture_t> dstTexture;
    TextureLayout dstLayout;
    std::vector<TextureResolveRegion> regions;
};

struct BufferUpdate {
    Handle<Buffer_t> dstBuffer;
    DeviceSize dstOffset{ 0 };
    const void *data{ nullptr };
    DeviceSize byteSize{ 0 };
};

struct BufferClear {
    Handle<Buffer_t> dstBuffer;
    DeviceSize dstOffset{ 0 };
    DeviceSize byteSize{ 0 };
    uint32_t clearValue{ 0 };
};

struct ClearColorTexture {
    Handle<Texture_t> texture;
    TextureLayout layout{ TextureLayout::ColorAttachmentOptimal };
    ColorClearValue clearValue;
    std::vector<TextureSubresourceRange> ranges;
};

struct ClearDepthStencilTexture {
    Handle<Texture_t> texture;
    TextureLayout layout{ TextureLayout::DepthStencilAttachmentOptimal };
    float depthClearValue{ 1.0f };
    uint32_t stencilClearValue{ 0 };
    std::vector<TextureSubresourceRange> ranges;
};

/*!
    \class CommandRecorder
    \brief Records GPU commands into a command buffer for submission
    \ingroup public
    \headerfile command_recorder.h <KDGpu/command_recorder.h>

    <b>Vulkan equivalent:</b> [VkCommandBuffer](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkCommandBuffer.html) (in recording state)

    CommandRecorder is the primary interface for recording GPU work. It provides methods to begin
    render passes, perform transfers, set barriers, and more. When finished, it produces a CommandBuffer
    that can be submitted to a Queue.

    <b>Key features:</b>
    - Begin render, compute, and ray tracing passes
    - Copy buffers and textures
    - Pipeline barriers and synchronization
    - Clear operations
    - Secondary command buffer execution
    - Debug labels and markers

    <br/>
    <b>Lifetime:</b> Create a CommandRecorder when you need to record work, then call finish() to get
    the CommandBuffer. The recorder itself can be discarded after finish().

    ## Usage

    <b>Basic command recording:</b>

    \snippet kdgpu_doc_snippets.cpp commandrecorder_creation

    <b>Recording render passes:</b>

    \snippet kdgpu_doc_snippets.cpp commandrecorder_render_pass

    <b>Compute pass recording:</b>

    \snippet kdgpu_doc_snippets.cpp commandrecorder_compute_pass

    <b>Buffer copies:</b>

    \snippet kdgpu_doc_snippets.cpp commandrecorder_copy_buffer

    <b>Texture copies:</b>

    \snippet kdgpu_doc_snippets.cpp commandrecorder_copy_texture

    <b>Submitting commands:</b>

    \snippet kdgpu_doc_snippets.cpp commandrecorder_submit

    <b>Recorder reuse:</b>

    \snippet kdgpu_doc_snippets.cpp commandrecorder_reuse

    ## Vulkan mapping:
    - CommandRecorder creation -> vkAllocateCommandBuffers() + vkBeginCommandBuffer()
    - CommandRecorder::finish() -> vkEndCommandBuffer()
    - CommandRecorder::copyBuffer() -> vkCmdCopyBuffer()
    - CommandRecorder::textureMemoryBarrier() -> vkCmdPipelineBarrier()
    - CommandRecorder::beginRenderPass() -> vkCmdBeginRenderPass()

    ## See also:
    \sa CommandRecorderOptions, CommandBuffer, RenderPassCommandRecorder, ComputePassCommandRecorder, Queue, Device
    \sa \ref kdgpu_api_overview
    \sa \ref kdgpu_vulkan_mapping
 */
class KDGPU_EXPORT CommandRecorder
{
public:
    ~CommandRecorder();

    CommandRecorder(CommandRecorder &&) noexcept;
    CommandRecorder &operator=(CommandRecorder &&) noexcept;

    CommandRecorder(const CommandRecorder &) = delete;
    CommandRecorder &operator=(const CommandRecorder &) = delete;

    const Handle<CommandRecorder_t> &handle() const noexcept { return m_commandRecorder; }
    bool isValid() const noexcept { return m_commandRecorder.isValid(); }

    operator Handle<CommandRecorder_t>() const noexcept { return m_commandRecorder; }

    RenderPassCommandRecorder beginRenderPass(const RenderPassCommandRecorderOptions &options) const;
    RenderPassCommandRecorder beginRenderPass(const RenderPassCommandRecorderWithRenderPassOptions &options) const;
    RenderPassCommandRecorder beginRenderPass(const RenderPassCommandRecorderWithDynamicRenderingOptions &options) const;

    [[nodiscard]] ComputePassCommandRecorder beginComputePass(const ComputePassCommandRecorderOptions &options = {}) const;
    [[nodiscard]] RayTracingPassCommandRecorder beginRayTracingPass(const RayTracingPassCommandRecorderOptions &options = {}) const;
    [[nodiscard]] TimestampQueryRecorder beginTimestampRecording(const TimestampQueryRecorderOptions &options = {}) const;
    void blitTexture(const TextureBlitOptions &options) const;
    void clearBuffer(const BufferClear &clear) const;
    void clearColorTexture(const ClearColorTexture &clear) const;
    void clearDepthStencilTexture(const ClearDepthStencilTexture &clear) const;
    void copyBuffer(const BufferCopy &copy) const;
    void copyBufferToTexture(const BufferToTextureCopy &copy) const;
    void copyTextureToBuffer(const TextureToBufferCopy &copy) const;
    void copyTextureToTexture(const TextureToTextureCopy &copy) const;
    void updateBuffer(const BufferUpdate &update) const;
    void memoryBarrier(const MemoryBarrierOptions &options) const;
    void bufferMemoryBarrier(const BufferMemoryBarrierOptions &options) const;
    void textureMemoryBarrier(const TextureMemoryBarrierOptions &options) const;
    void executeSecondaryCommandBuffer(const Handle<CommandBuffer_t> &secondaryCommandBuffer) const;
    void resolveTexture(const TextureResolveOptions &options) const;
    void buildAccelerationStructures(const BuildAccelerationStructureOptions &options) const;
    void beginDebugLabel(const DebugLabelOptions &options) const;
    void endDebugLabel() const;

    [[nodiscard]] CommandBuffer finish() const;

protected:
    explicit CommandRecorder(GraphicsApi *api, const Handle<Device_t> &device, const CommandRecorderOptions &options);

    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<CommandRecorder_t> m_commandRecorder;
    CommandBufferLevel m_level;
    // NOLINTEND(misc-non-private-member-variables-in-classes)

    friend class Device;
    friend class Queue;
};

} // namespace KDGpu
