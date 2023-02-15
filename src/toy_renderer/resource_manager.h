#pragma once

// TODO: Can we make these forward declarations?
#include <toy_renderer/adapter.h>
#include <toy_renderer/bind_group_description.h>
#include <toy_renderer/device.h>
#include <toy_renderer/gpu_semaphore.h>
#include <toy_renderer/handle.h>
#include <toy_renderer/pool.h>
#include <toy_renderer/queue.h>
#include <toy_renderer/swapchain.h>
#include <toy_renderer/surface.h>
#include <toy_renderer/texture.h>
#include <toy_renderer/texture_view.h>

#include <toy_renderer/toy_renderer_export.h>

namespace ToyRenderer {

struct ApiAdapter;
struct ApiBuffer;
struct ApiCommandBuffer;
struct ApiCommandRecorder;
struct ApiDevice;
struct ApiGpuSemaphore;
struct ApiGraphicsPipeline;
struct ApiInstance;
struct ApiPipelineLayout;
struct ApiQueue;
struct ApiShaderModule;
struct ApiSwapchain;
struct ApiSurface;
struct ApiTexture;
struct ApiTextureView;

class BindGroup;

struct BufferOptions;
struct CommandRecorderOptions;
struct DeviceOptions;
struct GpuSemaphoreOptions;
struct GraphicsPipelineOptions;
struct InstanceOptions;
struct PipelineLayoutOptions;
struct TextureOptions;
struct TextureViewOptions;

struct CommandRecorder_t;
struct GraphicsPipeline_t;
struct PipelineLayout_t;
struct ShaderModule_t;

// TODO: Should this class have create/destroy functions or should we put those onto the
// parent resource type structs? For example VulkanDevice could have a createTexture()
// function and just use this class as the place to store the resulting data.
class TOY_RENDERER_EXPORT ResourceManager
{
public:
    virtual ~ResourceManager();

    virtual Handle<Instance_t> createInstance(const InstanceOptions &options) = 0;
    virtual void deleteInstance(Handle<Instance_t> handle) = 0;
    virtual ApiInstance *getInstance(const Handle<Instance_t> &handle) = 0;

    // Adapters are not created, they are queried from the instance. It is up to
    // the concrete subclasses as to how they insert whatever they need.
    virtual void removeAdapter(Handle<Adapter_t> handle) = 0;
    virtual ApiAdapter *getAdapter(const Handle<Adapter_t> &handle) = 0;

    virtual Handle<Device_t> createDevice(const Handle<Adapter_t> &adapterHandle, const DeviceOptions &options, std::vector<QueueRequest> &queueRequests) = 0;
    virtual void deleteDevice(Handle<Device_t> handle) = 0;
    virtual ApiDevice *getDevice(const Handle<Device_t> &handle) = 0;

    // Queue are not created, they are queried from the device. It is up to
    // the concrete subclasses as to how they insert whatever they need.
    virtual void removeQueue(Handle<Queue_t> handle) = 0;
    virtual ApiQueue *getQueue(const Handle<Queue_t> &queue) = 0;

    // Surfaces are created by the api instance and inserted into the resource
    // manager by way of custom api on the api-specific resource manager concrete
    // classes.
    // TODO: Should we move the per-platform API here instead of on ApiInstance?
    // Or perhaps we should wrap up the per-platform options into a SurfaceOptions struct?
    // Then we could have something like:
    //
    // virtual Handle<Surface_t> createSurface(const Handle<Instance_t> &instanceHandle, const SurfaceOptions &options) = 0;
    virtual void deleteSurface(Handle<Surface_t> handle) = 0;
    virtual ApiSurface *getSurface(const Handle<Surface_t> &handle) = 0;

    virtual Handle<Swapchain_t> createSwapchain(const Handle<Device_t> &deviceHandle, const SwapchainOptions &options) = 0;
    virtual void deleteSwapchain(Handle<Swapchain_t> handle) = 0;
    virtual ApiSwapchain *getSwapchain(const Handle<Swapchain_t> &handle) = 0;

    virtual Handle<Texture_t> createTexture(const Handle<Device_t> deviceHandle, const TextureOptions &options) = 0;
    virtual void deleteTexture(Handle<Texture_t> handle) = 0;
    virtual ApiTexture *getTexture(const Handle<Texture_t> &handle) = 0;

    virtual Handle<TextureView_t> createTextureView(const Handle<Device_t> &deviceHandle, const Handle<Texture_t> &textureHandle, const TextureViewOptions &options) = 0;
    virtual void deleteTextureView(Handle<TextureView_t> handle) = 0;
    virtual ApiTextureView *getTextureView(const Handle<TextureView_t> &handle) = 0;

    virtual Handle<Buffer_t> createBuffer(const Handle<Device_t> deviceHandle, const BufferOptions &options, void *initialData) = 0;
    virtual void deleteBuffer(Handle<Buffer_t> handle) = 0;
    virtual ApiBuffer *getBuffer(const Handle<Buffer_t> &handle) = 0;

    virtual Handle<ShaderModule_t> createShaderModule(const Handle<Device_t> deviceHandle, const std::vector<uint32_t> &code) = 0;
    virtual void deleteShaderModule(Handle<ShaderModule_t> handle) = 0;
    virtual ApiShaderModule *getShaderModule(const Handle<ShaderModule_t> &handle) = 0;

    virtual Handle<PipelineLayout_t> createPipelineLayout(const Handle<Device_t> &deviceHandle, const PipelineLayoutOptions &options) = 0;
    virtual void deletePipelineLayout(Handle<PipelineLayout_t> handle) = 0;
    virtual ApiPipelineLayout *getPipelineLayout(const Handle<PipelineLayout_t> &handle) = 0;

    virtual Handle<GraphicsPipeline_t> createGraphicsPipeline(const Handle<Device_t> &deviceHandle, const GraphicsPipelineOptions &options) = 0;
    virtual void deleteGraphicsPipeline(Handle<GraphicsPipeline_t> handle) = 0;
    virtual ApiGraphicsPipeline *getGraphicsPipeline(const Handle<GraphicsPipeline_t> &handle) = 0;

    virtual Handle<GpuSemaphore_t> createGpuSemaphore(const Handle<Device_t> &deviceHandle, const GpuSemaphoreOptions &options) = 0;
    virtual void deleteGpuSemaphore(Handle<GpuSemaphore_t> handle) = 0;
    virtual ApiGpuSemaphore *getGpuSemaphore(const Handle<GpuSemaphore_t> &handle) = 0;

    virtual Handle<CommandRecorder_t> createCommandRecorder(const Handle<Device_t> &deviceHandle, const CommandRecorderOptions &options) = 0;
    virtual void deleteCommandRecorder(Handle<CommandRecorder_t> handle) = 0;
    virtual ApiCommandRecorder *getCommandRecorder(const Handle<CommandRecorder_t> &handle) = 0;

    // Command buffers are not created by the api. It is up to the concrete subclasses to insert the command buffers
    // by whatever mechanism they wish. They also do not need to be destroyed as they are cleaned up by the owning
    // command pool (command recorder).
    virtual ApiCommandBuffer *getCommandBuffer(const Handle<CommandBuffer_t> &handle) = 0;

    // virtual Handle<Shader> createShader(ShaderDescription desc) = 0;
    virtual Handle<BindGroup> createBindGroup(BindGroupDescription desc) = 0;
    // virtual Handle<Texture> createTexture(TextureDescription desc) = 0;
    // virtual Handle<Buffer> createBuffer(BufferDescription desc) = 0;

    // virtual void deleteShader(Handle<Shader> handle) = 0;
    virtual void deleteBindGroup(Handle<BindGroup> handle) = 0;
    // virtual void deleteTexture(Handle<Texture> handle) = 0;
    // virtual void deleteBuffer(Handle<Buffer> handle) = 0;

protected:
    ResourceManager();
};

} // namespace ToyRenderer
