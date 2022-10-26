#pragma once

#include <toy_renderer/adapter.h>
#include <toy_renderer/bind_group_description.h>
#include <toy_renderer/handle.h>
#include <toy_renderer/instance.h>
#include <toy_renderer/pool.h>

#include <toy_renderer/toy_renderer_export.h>

namespace ToyRenderer {

struct ApiAdapter;
struct ApiInstance;
class BindGroup;
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
