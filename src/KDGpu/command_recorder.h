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
