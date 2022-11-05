#pragma once

#include <toy_renderer/adapter.h>
#include <toy_renderer/bind_group_description.h>
#include <toy_renderer/device.h>
#include <toy_renderer/handle.h>
#include <toy_renderer/pool.h>
#include <toy_renderer/queue.h>
#include <toy_renderer/swapchain.h>
#include <toy_renderer/surface.h>

#include <toy_renderer/toy_renderer_export.h>

namespace ToyRenderer {

struct ApiAdapter;
struct ApiDevice;
struct ApiInstance;
struct ApiQueue;
struct ApiSwapchain;
struct ApiSurface;

class BindGroup;

struct InstanceOptions;
struct DeviceOptions;

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
