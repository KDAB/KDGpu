#include "instance.h"

#include <toy_renderer/handle.h>
#include <toy_renderer/pool.h>

namespace Gpu {

class ResourceManager
{
public:
    static ResourceManager *instance() { return ms_instance; }

    virtual ~ResourceManager() { }

    virtual Handle<Instance_t> createInstance(const InstanceOptions &options) = 0;
    virtual void deleteInstance(Handle<Instance_t> handle) = 0;

    // virtual Handle<Shader> createShader(ShaderDescription desc) = 0;
    // virtual Handle<BindGroup> createBindGroup(BindGroupDescription desc) = 0;
    // virtual Handle<Texture> createTexture(TextureDescription desc) = 0;
    // virtual Handle<Buffer> createBuffer(BufferDescription desc) = 0;

    // virtual void deleteShader(Handle<Shader> handle) = 0;
    // virtual void deleteBindGroup(Handle<BindGroup> handle) = 0;
    // virtual void deleteTexture(Handle<Texture> handle) = 0;
    // virtual void deleteBuffer(Handle<Buffer> handle) = 0;

protected:
    static ResourceManager *ms_instance;
};

} // namespace Gpu
