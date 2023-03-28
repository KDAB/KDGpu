#pragma once

// TODO: Can we make these forward declarations?
#include <kdgpu/adapter.h>
#include <kdgpu/bind_group.h>
#include <kdgpu/bind_group_description.h>
#include <kdgpu/device.h>
#include <kdgpu/gpu_semaphore.h>
#include <kdgpu/handle.h>
#include <kdgpu/pool.h>
#include <kdgpu/queue.h>
#include <kdgpu/swapchain.h>
#include <kdgpu/surface.h>
#include <kdgpu/texture.h>
#include <kdgpu/texture_view.h>
#include <kdgpu/fence.h>

#include <kdgpu/kdgpu_export.h>

namespace KDGpu {

struct ApiAdapter;
struct ApiBindGroup;
struct ApiBindGroupLayout;
struct ApiBuffer;
struct ApiCommandBuffer;
struct ApiCommandRecorder;
struct ApiComputePipeline;
struct ApiComputePassCommandRecorder;
struct ApiDevice;
struct ApiFence;
struct ApiGpuSemaphore;
struct ApiGraphicsPipeline;
struct ApiInstance;
struct ApiPipelineLayout;
struct ApiQueue;
struct ApiRenderPass;
struct ApiRenderPassCommandRecorder;
struct ApiSampler;
struct ApiShaderModule;
struct ApiSwapchain;
struct ApiSurface;
struct ApiTexture;
struct ApiTextureView;

struct BindGroupOptions;
struct BufferOptions;
struct CommandRecorderOptions;
struct ComputePipelineOptions;
struct DeviceOptions;
struct FenceOptions;
struct GpuSemaphoreOptions;
struct GraphicsPipelineOptions;
struct InstanceOptions;
struct PipelineLayoutOptions;
struct RenderPassCommandRecorderOptions;
struct SamplerOptions;
struct TextureOptions;
struct TextureViewOptions;

struct BindGroup_t;
struct CommandRecorder_t;
struct ComputePipeline_t;
struct Fence_t;
struct GraphicsPipeline_t;
struct PipelineLayout_t;
struct RenderPass_t;
struct Sampler_t;
struct ShaderModule_t;

// TODO: Should this class have create/destroy functions or should we put those onto the
// parent resource type structs? For example VulkanDevice could have a createTexture()
// function and just use this class as the place to store the resulting data.
class KDGPU_EXPORT ResourceManager
{
public:
    virtual ~ResourceManager();

    virtual Handle<Instance_t> createInstance(const InstanceOptions &options) = 0;
    virtual void deleteInstance(const Handle<Instance_t> &handle) = 0;
    virtual ApiInstance *getInstance(const Handle<Instance_t> &handle) const = 0;

    // Adapters are not created, they are queried from the instance. It is up to
    // the concrete subclasses as to how they insert whatever they need.
    virtual void removeAdapter(const Handle<Adapter_t> &handle) = 0;
    virtual ApiAdapter *getAdapter(const Handle<Adapter_t> &handle) const = 0;

    virtual Handle<Device_t> createDevice(const Handle<Adapter_t> &adapterHandle, const DeviceOptions &options, std::vector<QueueRequest> &queueRequests) = 0;
    virtual void deleteDevice(const Handle<Device_t> &handle) = 0;
    virtual ApiDevice *getDevice(const Handle<Device_t> &handle) const = 0;

    // Queue are not created, they are queried from the device. It is up to
    // the concrete subclasses as to how they insert whatever they need.
    virtual void removeQueue(const Handle<Queue_t> &handle) = 0;
    virtual ApiQueue *getQueue(const Handle<Queue_t> &queue) const = 0;

    // Surfaces are created by the api instance and inserted into the resource
    // manager by way of custom api on the api-specific resource manager concrete
    // classes.
    // TODO: Should we move the per-platform API here instead of on ApiInstance?
    // Or perhaps we should wrap up the per-platform options into a SurfaceOptions struct?
    // Then we could have something like:
    //
    // virtual Handle<Surface_t> createSurface(const Handle<Instance_t> &instanceHandle, const SurfaceOptions &options) = 0;
    virtual void deleteSurface(const Handle<Surface_t> &handle) = 0;
    virtual ApiSurface *getSurface(const Handle<Surface_t> &handle) const = 0;

    virtual Handle<Swapchain_t> createSwapchain(const Handle<Device_t> &deviceHandle, const SwapchainOptions &options) = 0;
    virtual void deleteSwapchain(const Handle<Swapchain_t> &handle) = 0;
    virtual ApiSwapchain *getSwapchain(const Handle<Swapchain_t> &handle) const = 0;

    virtual Handle<Texture_t> createTexture(const Handle<Device_t> &deviceHandle, const TextureOptions &options) = 0;
    virtual void deleteTexture(const Handle<Texture_t> &handle) = 0;
    virtual ApiTexture *getTexture(const Handle<Texture_t> &handle) const = 0;

    virtual Handle<TextureView_t> createTextureView(const Handle<Device_t> &deviceHandle, const Handle<Texture_t> &textureHandle, const TextureViewOptions &options) = 0;
    virtual void deleteTextureView(const Handle<TextureView_t> &handle) = 0;
    virtual ApiTextureView *getTextureView(const Handle<TextureView_t> &handle) const = 0;

    virtual Handle<Buffer_t> createBuffer(const Handle<Device_t> &deviceHandle, const BufferOptions &options, const void *initialData) = 0;
    virtual void deleteBuffer(const Handle<Buffer_t> &handle) = 0;
    virtual ApiBuffer *getBuffer(const Handle<Buffer_t> &handle) const = 0;

    virtual Handle<ShaderModule_t> createShaderModule(const Handle<Device_t> &deviceHandle, const std::vector<uint32_t> &code) = 0;
    virtual void deleteShaderModule(const Handle<ShaderModule_t> &handle) = 0;
    virtual ApiShaderModule *getShaderModule(const Handle<ShaderModule_t> &handle) const = 0;

    virtual Handle<PipelineLayout_t> createPipelineLayout(const Handle<Device_t> &deviceHandle, const PipelineLayoutOptions &options) = 0;
    virtual void deletePipelineLayout(const Handle<PipelineLayout_t> &handle) = 0;
    virtual ApiPipelineLayout *getPipelineLayout(const Handle<PipelineLayout_t> &handle) const = 0;

    virtual Handle<GraphicsPipeline_t> createGraphicsPipeline(const Handle<Device_t> &deviceHandle, const GraphicsPipelineOptions &options) = 0;
    virtual void deleteGraphicsPipeline(const Handle<GraphicsPipeline_t> &handle) = 0;
    virtual ApiGraphicsPipeline *getGraphicsPipeline(const Handle<GraphicsPipeline_t> &handle) const = 0;

    virtual Handle<ComputePipeline_t> createComputePipeline(const Handle<Device_t> &deviceHandle, const ComputePipelineOptions &options) = 0;
    virtual void deleteComputePipeline(const Handle<ComputePipeline_t> &handle) = 0;
    virtual ApiComputePipeline *getComputePipeline(const Handle<ComputePipeline_t> &handle) const = 0;

    virtual Handle<GpuSemaphore_t> createGpuSemaphore(const Handle<Device_t> &deviceHandle, const GpuSemaphoreOptions &options) = 0;
    virtual void deleteGpuSemaphore(const Handle<GpuSemaphore_t> &handle) = 0;
    virtual ApiGpuSemaphore *getGpuSemaphore(const Handle<GpuSemaphore_t> &handle) const = 0;

    virtual Handle<CommandRecorder_t> createCommandRecorder(const Handle<Device_t> &deviceHandle, const CommandRecorderOptions &options) = 0;
    virtual void deleteCommandRecorder(const Handle<CommandRecorder_t> &handle) = 0;
    virtual ApiCommandRecorder *getCommandRecorder(const Handle<CommandRecorder_t> &handle) const = 0;

    virtual Handle<RenderPassCommandRecorder_t> createRenderPassCommandRecorder(const Handle<Device_t> &deviceHandle,
                                                                                const Handle<CommandRecorder_t> &commandRecorderHandle,
                                                                                const RenderPassCommandRecorderOptions &options) = 0;
    virtual void deleteRenderPassCommandRecorder(const Handle<RenderPassCommandRecorder_t> &handle) = 0;
    virtual ApiRenderPassCommandRecorder *getRenderPassCommandRecorder(const Handle<RenderPassCommandRecorder_t> &handle) const = 0;

    virtual Handle<ComputePassCommandRecorder_t> createComputePassCommandRecorder(const Handle<Device_t> &deviceHandle,
                                                                                  const Handle<CommandRecorder_t> &commandRecorderHandle,
                                                                                  const ComputePassCommandRecorderOptions &options) = 0;
    virtual void deleteComputePassCommandRecorder(const Handle<ComputePassCommandRecorder_t> &handle) = 0;
    virtual ApiComputePassCommandRecorder *getComputePassCommandRecorder(const Handle<ComputePassCommandRecorder_t> &handle) const = 0;

    virtual Handle<CommandBuffer_t> createCommandBuffer(const Handle<Device_t> &deviceHandle,
                                                        const QueueDescription &queueDescription,
                                                        CommandBufferLevel commandLevel) = 0;
    virtual void deleteCommandBuffer(const Handle<CommandBuffer_t> &handle) = 0;
    virtual ApiCommandBuffer *getCommandBuffer(const Handle<CommandBuffer_t> &handle) const = 0;

    // virtual Handle<Shader> createShader(ShaderDescription desc) = 0;
    virtual Handle<BindGroup_t> createBindGroup(const Handle<Device_t> &deviceHandle, const BindGroupOptions &options) = 0;
    virtual void deleteBindGroup(const Handle<BindGroup_t> &handle) = 0;
    virtual ApiBindGroup *getBindGroup(const Handle<BindGroup_t> &handle) const = 0;

    virtual Handle<BindGroupLayout_t> createBindGroupLayout(const Handle<Device_t> &deviceHandle, const BindGroupLayoutOptions &options) = 0;
    virtual void deleteBindGroupLayout(const Handle<BindGroupLayout_t> &handle) = 0;
    virtual ApiBindGroupLayout *getBindGroupLayout(const Handle<BindGroupLayout_t> &handle) const = 0;

    virtual Handle<Sampler_t> createSampler(const Handle<Device_t> &deviceHandle, const SamplerOptions &options) = 0;
    virtual void deleteSampler(const Handle<Sampler_t> &handle) = 0;
    virtual ApiSampler *getSampler(const Handle<Sampler_t> &handle) const = 0;

    virtual Handle<Fence_t> createFence(const Handle<Device_t> &deviceHandle, const FenceOptions &options) = 0;
    virtual void deleteFence(const Handle<Fence_t> &handle) = 0;
    virtual ApiFence *getFence(const Handle<Fence_t> &handle) const = 0;

protected:
    ResourceManager();
};

} // namespace KDGpu
