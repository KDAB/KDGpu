/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/handle.h>
#include <KDGpu/memory_barrier.h>
#include <KDGpu/render_pass_command_recorder_options.h>

namespace KDGpu {

struct CommandBuffer_t;
struct RenderPassCommandRecorder_t;
struct MemoryBarrierOptions;
struct BufferCopy;
struct BufferToTextureCopy;
struct TextureToBufferCopy;
struct TextureToTextureCopy;

/**
 * @brief ApiCommandRecorder
 *
 */
struct ApiCommandRecorder {
    virtual void begin() = 0;
    virtual void copyBuffer(const BufferCopy &copy) = 0;
    virtual void copyBufferToTexture(const BufferToTextureCopy &copy) = 0;
    virtual void copyTextureToBuffer(const TextureToBufferCopy &copy) = 0;
    virtual void copyTextureToTexture(const TextureToTextureCopy &copy) = 0;
    virtual void memoryBarrier(const MemoryBarrierOptions &options) = 0;
    virtual void bufferMemoryBarrier(const BufferMemoryBarrierOptions &options) = 0;
    virtual void textureMemoryBarrier(const TextureMemoryBarrierOptions &options) = 0;
    virtual void executeSecondaryCommandBuffer(const Handle<CommandBuffer_t> &secondaryCommandBuffer) = 0;
    virtual Handle<CommandBuffer_t> finish() = 0;
};

} // namespace KDGpu
