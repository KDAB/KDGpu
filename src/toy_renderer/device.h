#pragma once

#include <toy_renderer/bind_group.h>
#include <toy_renderer/bind_group_layout.h>
#include <toy_renderer/buffer.h>
#include <toy_renderer/command_recorder.h>
#include <toy_renderer/compute_pipeline.h>
#include <toy_renderer/device_options.h>
#include <toy_renderer/gpu_semaphore.h>
#include <toy_renderer/graphics_pipeline.h>
#include <toy_renderer/handle.h>
#include <toy_renderer/pipeline_layout.h>
#include <toy_renderer/pipeline_layout_options.h>
#include <toy_renderer/queue.h>
#include <toy_renderer/sampler.h>
#include <toy_renderer/shader_module.h>
#include <toy_renderer/swapchain.h>

#include <toy_renderer/toy_renderer_export.h>

#include <span>
#include <string>
#include <vector>

namespace ToyRenderer {

class Adapter;
class GraphicsApi;

struct Adapter_t;
struct Device_t;

struct BufferOptions;
struct GraphicsPipelineOptions;
struct SamplerOptions;
struct SwapchainOptions;
struct TextureOptions;
struct BindGroupOptions;
struct BindGroupLayoutOptions;
struct BindGroupEntry;
struct ComputePipelineOptions;

class TOY_RENDERER_EXPORT Device
{
public:
    Device();
    ~Device();

    Handle<Device_t> handle() const noexcept { return m_device; }
    bool isValid() const noexcept { return m_device.isValid(); }

    operator Handle<Device_t>() const noexcept { return m_device; }

    std::span<Queue> queues() { return m_queues; }

    void waitUntilIdle();

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

    Sampler createSampler(const SamplerOptions &options);

private:
    Device(Adapter *adapter, GraphicsApi *api, const DeviceOptions &options);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    std::vector<Queue> m_queues;

    friend class Adapter;
};

} // namespace ToyRenderer
