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

struct BufferImageCopyRegion {
    DeviceSize bufferOffset{ 0 };
    uint32_t bufferRowLength{ 0 };
    uint32_t bufferImageHeight{ 0 };
    TextureSubresourceLayers imageSubResource{};
    Offset3D imageOffset{};
    Extent3D imageExtent{};
};

struct BufferToTextureCopy {
    Handle<Buffer_t> srcBuffer;
    Handle<Texture_t> dstTexture;
    TextureLayout dstImageLayout;
    std::vector<BufferImageCopyRegion> regions;
};

struct TextureToBufferCopy {
    Handle<Texture_t> srcTexture;
    TextureLayout srcImageLayout;
    Handle<Buffer_t> dstBuffer;
    std::vector<BufferImageCopyRegion> regions;
};

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
    void copyBuffer(const BufferCopy &copy);
    void copyBufferToTexture(const BufferToTextureCopy &copy);
    void copyTextureToBuffer(const TextureToBufferCopy &copy);
    void memoryBarrier(const MemoryBarrierOptions &options);
    void bufferMemoryBarrier(const BufferMemoryBarrierOptions &options);
    void textureMemoryBarrier(const TextureMemoryBarrierOptions &options);
    void executeSecondaryCommandBuffer(const Handle<CommandBuffer_t> &secondaryCommandBuffer);

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
