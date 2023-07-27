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
#include <KDGpu/render_pass_command_recorder_options.h>
#include <KDGpu/kdgpu_export.h>
#include <KDGpu/memory_barrier.h>

namespace KDGpu {

class GraphicsApi;

struct CommandRecorder_t;
struct Device_t;
struct Queue_t;

struct CommandRecorderOptions {
    Handle<Queue_t> queue; // The queue on which you wish to submit the recorded commands. If not set, defaults to first queue of the device
    CommandBufferLevel level{ CommandBufferLevel::Primary };
};

struct BufferCopy {
    Handle<Buffer_t> src;
    size_t srcOffset{ 0 };
    Handle<Buffer_t> dst;
    size_t dstOffset{ 0 };
    size_t byteSize{ 0 };
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

/**
 * @brief CommandRecorder
 * @ingroup public
 */
class KDGPU_EXPORT CommandRecorder
{
public:
    ~CommandRecorder();

    CommandRecorder(CommandRecorder &&);
    CommandRecorder &operator=(CommandRecorder &&);

    CommandRecorder(const CommandRecorder &) = delete;
    CommandRecorder &operator=(const CommandRecorder &) = delete;

    const Handle<CommandRecorder_t> &handle() const noexcept { return m_commandRecorder; }
    bool isValid() const noexcept { return m_commandRecorder.isValid(); }

    operator Handle<CommandRecorder_t>() const noexcept { return m_commandRecorder; }

    RenderPassCommandRecorder beginRenderPass(const RenderPassCommandRecorderOptions &options);
    ComputePassCommandRecorder beginComputePass(const ComputePassCommandRecorderOptions &options = {});
    void blitTexture(const TextureBlitOptions &options);
    void clearBuffer(const BufferClear &clear);
    void clearColorTexture(const ClearColorTexture &clear);
    void clearDepthStencilTexture(const ClearDepthStencilTexture &clear);
    void copyBuffer(const BufferCopy &copy);
    void copyBufferToTexture(const BufferToTextureCopy &copy);
    void copyTextureToBuffer(const TextureToBufferCopy &copy);
    void copyTextureToTexture(const TextureToTextureCopy &copy);
    void updateBuffer(const BufferUpdate &update);
    void memoryBarrier(const MemoryBarrierOptions &options);
    void bufferMemoryBarrier(const BufferMemoryBarrierOptions &options);
    void textureMemoryBarrier(const TextureMemoryBarrierOptions &options);
    void executeSecondaryCommandBuffer(const Handle<CommandBuffer_t> &secondaryCommandBuffer);
    void resolveTexture(const TextureResolveOptions &options);

    CommandBuffer finish();

protected:
    explicit CommandRecorder(GraphicsApi *api, const Handle<Device_t> &device, const CommandRecorderOptions &options);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<CommandRecorder_t> m_commandRecorder;
    CommandBufferLevel m_level;

    friend class Device;
    friend class Queue;
};

} // namespace KDGpu
