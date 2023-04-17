#pragma once

#include <KDGpu/buffer.h>
#include <KDGpu/command_buffer.h>
#include <KDGpu/command_recorder.h>
#include <KDGpu/fence.h>
#include <KDGpu/gpu_core.h>

#include <KDGpu/kdgpu_export.h>

namespace KDGpu {

class Device;
class Queue;
struct Texture_t;

struct UploadStagingBuffer {
    Fence fence;
    Buffer buffer;
    CommandBuffer commandBuffer;
};

KDGPU_EXPORT void waitForUploadBufferData(Device *device,
                                          Queue *queue,
                                          const Handle<Buffer_t> &destinationBuffer,
                                          const void *data,
                                          DeviceSize byteSize,
                                          DeviceSize dstOffset = 0);

KDGPU_EXPORT UploadStagingBuffer uploadBufferData(Device *device,
                                                  Queue *queue,
                                                  const Handle<Buffer_t> &destinationBuffer,
                                                  PipelineStageFlags dstStages,
                                                  AccessFlags dstMask,
                                                  const void *data,
                                                  DeviceSize byteSize,
                                                  DeviceSize dstOffset = 0);

KDGPU_EXPORT void waitForUploadTextureData(Device *device,
                                           Queue *queue,
                                           const Handle<Texture_t> &destinationTexture,
                                           const void *data,
                                           DeviceSize byteSize,
                                           TextureLayout oldLayout,
                                           TextureLayout newLayout,
                                           const std::vector<BufferImageCopyRegion> &regions);

KDGPU_EXPORT UploadStagingBuffer uploadTextureData(Device *device,
                                                   Queue *queue,
                                                   const Handle<Texture_t> &destinationTexture,
                                                   PipelineStageFlags dstStages,
                                                   AccessFlags dstMask,
                                                   const void *data,
                                                   DeviceSize byteSize,
                                                   TextureLayout oldLayout,
                                                   TextureLayout newLayout,
                                                   const std::vector<BufferImageCopyRegion> &regions);

} // namespace KDGpu
