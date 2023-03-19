#pragma once

#include <toy_renderer/handle.h>
#include <toy_renderer/gpu_core.h>

namespace ToyRenderer {

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
    uint32_t srcQueueTypeIndex{ 0 };
    uint32_t dstQueueTypeIndex{ 0 };
    Handle<Buffer_t> buffer;
    DeviceSize offset{ 0 };
    DeviceSize size{ WholeSize };
};

struct TextureMemoryBarrierOptions {
    PipelineStageFlags srcStages;
    AccessFlags srcMask;
    PipelineStageFlags dstStages;
    AccessFlags dstMask;
    TextureLayout oldLayout{ TextureLayout::Undefined };
    TextureLayout newLayout{ TextureLayout::Undefined };
    uint32_t srcQueueTypeIndex{ 0 };
    uint32_t dstQueueTypeIndex{ 0 };
    Handle<Texture_t> texture;
    TextureSubresourceRange range{};
};

} // namespace ToyRenderer
