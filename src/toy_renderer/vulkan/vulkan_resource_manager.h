#pragma once

#include <toy_renderer/resource_manager.h>

namespace ToyRenderer {

class TOY_RENDERER_EXPORT VulkanResourceManager : public ResourceManager
{
public:
    VulkanResourceManager();
    ~VulkanResourceManager() override;

    // Handle<Shader> createShader(ShaderDescription desc) override;
    Handle<BindGroup> createBindGroup(BindGroupDescription desc) override;
    // Handle<Texture> createTexture(TextureDescription desc) override;
    // Handle<Buffer> createBuffer(BufferDescription desc) override;

    // void deleteShader(Handle<Shader> handle) override;
    void deleteBindGroup(Handle<BindGroup> handle) override;
    // void deleteTexture(Handle<Texture> handle) override;
    // void deleteBuffer(Handle<Buffer> handle) override;
};

} // namespace ToyRenderer
