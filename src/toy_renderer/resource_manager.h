#pragma once

#include <toy_renderer/toy_renderer_export.h>
#include <toy_renderer/bind_group_description.h>
#include <toy_renderer/handle.h>

namespace ToyRenderer {
    
class BindGroup;

class TOY_RENDERER_EXPORT ResourceManager
{
public:
    ResourceManager();
    virtual ~ResourceManager();
    
    // virtual Handle<Shader> createShader(ShaderDescription desc) = 0;
    virtual Handle<BindGroup> createBindGroup(BindGroupDescription desc) = 0;
    // virtual Handle<Texture> createTexture(TextureDescription desc) = 0;
    // virtual Handle<Buffer> createBuffer(BufferDescription desc) = 0;

    // virtual void deleteShader(Handle<Shader> handle) = 0;
    virtual void deleteBindGroup(Handle<BindGroup> handle) = 0;
    // virtual void deleteTexture(Handle<Texture> handle) = 0;
    // virtual void deleteBuffer(Handle<Buffer> handle) = 0;
};

}
