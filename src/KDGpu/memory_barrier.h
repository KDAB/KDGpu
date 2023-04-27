/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/handle.h>
#include <KDGpu/gpu_core.h>

#include <vector>

namespace KDGpu {

struct Buffer_t;
struct Texture_t;

// TODO: Deprecate and remove these in favour of those more like VK_KHR_synchronization2
struct MemoryBarrier {
    AccessFlags srcMask;
    AccessFlags dstMask;
};

struct MemoryBarrierOptions {
    PipelineStageFlags srcStages;
    PipelineStageFlags dstStages;
    std::vector<MemoryBarrier> memoryBarriers;
};

// The new stuff
struct BufferMemoryBarrierOptions {
    PipelineStageFlags srcStages;
    AccessFlags srcMask;
    PipelineStageFlags dstStages;
    AccessFlags dstMask;
    uint32_t srcQueueTypeIndex{ IgnoreQueueType };
    uint32_t dstQueueTypeIndex{ IgnoreQueueType };
    Handle<Buffer_t> buffer;
    DeviceSize offset{ 0 };
    DeviceSize size{ WholeSize };
};

struct TextureMemoryBarrierOptions {
    PipelineStageFlags srcStages;
    AccessFlags srcMask{ AccessFlags(AccessFlagBit::None) };
    PipelineStageFlags dstStages;
    AccessFlags dstMask{ AccessFlags(AccessFlagBit::None) };
    TextureLayout oldLayout{ TextureLayout::Undefined };
    TextureLayout newLayout{ TextureLayout::Undefined };
    uint32_t srcQueueTypeIndex{ IgnoreQueueType };
    uint32_t dstQueueTypeIndex{ IgnoreQueueType };
    Handle<Texture_t> texture;
    TextureSubresourceRange range{};
};

} // namespace KDGpu
