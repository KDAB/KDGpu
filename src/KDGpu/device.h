#pragma once

#include <KDGpu/bind_group.h>
#include <KDGpu/bind_group_layout.h>
#include <KDGpu/buffer.h>
#include <KDGpu/command_recorder.h>
#include <KDGpu/compute_pipeline.h>
#include <KDGpu/device_options.h>
#include <KDGpu/fence.h>
#include <KDGpu/gpu_semaphore.h>
#include <KDGpu/graphics_pipeline.h>
#include <KDGpu/handle.h>
#include <KDGpu/pipeline_layout.h>
#include <KDGpu/pipeline_layout_options.h>
#include <KDGpu/queue.h>
#include <KDGpu/sampler.h>
#include <KDGpu/sampler_options.h>
#include <KDGpu/shader_module.h>
#include <KDGpu/swapchain.h>

#include <KDGpu/kdgpu_export.h>

#include <span>
#include <string>
#include <vector>

namespace KDGpu {

class GraphicsApi;
class Adapter;

struct Device_t;

struct BufferOptions;
struct GraphicsPipelineOptions;
struct SwapchainOptions;
struct TextureOptions;
struct BindGroupOptions;
struct BindGroupLayoutOptions;
struct BindGroupEntry;
struct ComputePipelineOptions;

class KDGPU_EXPORT Device
{
public:
    Device();
    ~Device();

    Device(Device &&);
    Device &operator=(Device &&);

    Device(const Device &) = delete;
    Device &operator=(const Device &) = delete;

    Handle<Device_t> handle() const noexcept { return m_device; }
    bool isValid() const noexcept { return m_device.isValid(); }

    operator Handle<Device_t>() const noexcept { return m_device; }

    std::span<Queue> queues() { return m_queues; }

    void waitUntilIdle();

    const Adapter *adapter() const;

    Swapchain createSwapchain(const SwapchainOptions &options);
    Texture createTexture(const TextureOptions &options);

    // TODO: If initialData is set, upload this to the newly created buffer.
    // OR should this helper functionality go in a slightly higher layer that
    // knows about the concept of a frame so that it can correctly submit such commands
    // as part of the frame submission along with suitable memory barriers?
    Buffer createBuffer(const BufferOptions &options, const void *initialData = nullptr);

    ShaderModule createShaderModule(const std::vector<uint32_t> &code);

    PipelineLayout createPipelineLayout(const PipelineLayoutOptions &options = PipelineLayoutOptions());

    GraphicsPipeline createGraphicsPipeline(const GraphicsPipelineOptions &options);

    ComputePipeline createComputePipeline(const ComputePipelineOptions &options);

    CommandRecorder createCommandRecorder(const CommandRecorderOptions &options = CommandRecorderOptions());

    GpuSemaphore createGpuSemaphore(const GpuSemaphoreOptions &options = GpuSemaphoreOptions());

    BindGroupLayout createBindGroupLayout(const BindGroupLayoutOptions &options);

    BindGroup createBindGroup(const BindGroupOptions &options);

    Sampler createSampler(const SamplerOptions &options = SamplerOptions());

    Fence createFence(const FenceOptions &options = FenceOptions());

    GraphicsApi *graphicsApi() const;

private:
    Device(Adapter *adapter, GraphicsApi *api, const DeviceOptions &options);

    GraphicsApi *m_api{ nullptr };
    Adapter *m_adapter{ nullptr };
    Handle<Device_t> m_device;
    std::vector<Queue> m_queues;

    friend class Adapter;
};

} // namespace KDGpu
