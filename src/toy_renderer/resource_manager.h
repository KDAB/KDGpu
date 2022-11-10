#pragma once

// TODO: Can we make these forward declarations?
#include <toy_renderer/adapter.h>
#include <toy_renderer/bind_group_description.h>
#include <toy_renderer/device.h>
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
struct ApiDevice;
struct ApiInstance;
struct ApiQueue;
struct ApiSwapchain;
struct ApiSurface;
struct ApiTexture;
struct ApiTextureView;

class BindGroup;

struct InstanceOptions;
struct DeviceOptions;
struct TextureViewOptions;

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

    virtual ApiTexture *getTexture(const Handle<Texture_t> &handle) = 0;

    virtual Handle<TextureView_t> createTextureView(const Handle<Device_t> &deviceHandle, const Handle<Texture_t> &textureHandle, const TextureViewOptions &options) = 0;
    virtual void deleteTextureView(Handle<TextureView_t> handle) = 0;
    virtual ApiTextureView *getTextureView(const Handle<TextureView_t> &handle) = 0;

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
