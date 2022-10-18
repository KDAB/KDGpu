#pragma once

#include "../resource_manager.h"

#include <toy_renderer/pool.h>

#include <vulkan/vulkan.h>

namespace Gpu {

class VulkanResourceManager : public ResourceManager
{
public:
    VulkanResourceManager();
    ~VulkanResourceManager() final;

    Handle<Instance_t> createInstance(const InstanceOptions &options) final;
    void deleteInstance(Handle<Instance_t> handle) final;

    // virtual Handle<Shader> createShader(ShaderDescription desc) = 0;
    // virtual Handle<BindGroup> createBindGroup(BindGroupDescription desc) = 0;
    // virtual Handle<Texture> createTexture(TextureDescription desc) = 0;
    // virtual Handle<Buffer> createBuffer(BufferDescription desc) = 0;

    // virtual void deleteShader(Handle<Shader> handle) = 0;
    // virtual void deleteBindGroup(Handle<BindGroup> handle) = 0;
    // virtual void deleteTexture(Handle<Texture> handle) = 0;
    // virtual void deleteBuffer(Handle<Buffer> handle) = 0;

private:
    Pool<VkInstance, Instance_t> m_instances{ 1 };
};

} // namespace Gpu
