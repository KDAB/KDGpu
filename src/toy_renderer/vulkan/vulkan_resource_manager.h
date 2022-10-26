#pragma once

#include <toy_renderer/resource_manager.h>

#include <toy_renderer/pool.h>
#include <toy_renderer/vulkan/vulkan_adapter.h>
#include <toy_renderer/vulkan/vulkan_instance.h>

#include <toy_renderer/toy_renderer_export.h>

#include <vulkan/vulkan.h>

// Should we refactor this into more smaller classes like the platform integration? For example,
// should we have PlatformInstance to backend Instance and that class has the functionality to
// query/enumerate VkPhysicalDevices? As opposed to storing a plain VkInstance as we do now.
// Could look something like this:
//
// struct PlatformInstance {
//     VkInstance instance;
//     std::vector<VkPhysicalDevice> queryPhysicalDevices()
//     {
//         vkEnumeratePhysicalDevices(...);
//         ...;
//     }
// };
//
// That would avoid having too much logic directly in the VulkanGraphicsApi class and this
// class.
//
// TODO: Try the above to see how it looks once we have something working.

namespace ToyRenderer {

class TOY_RENDERER_EXPORT VulkanResourceManager : public ResourceManager
{
public:
    VulkanResourceManager();
    ~VulkanResourceManager() final;

    Handle<Instance_t> createInstance(const InstanceOptions &options) final;
    void deleteInstance(Handle<Instance_t> handle) final;
    VulkanInstance *getInstance(const Handle<Instance_t> &handle) { return m_instances.get(handle); }

    Handle<Adapter_t> insertAdapter(const VulkanAdapter &vulkanAdapter);
    void removeAdapter(Handle<Adapter_t> handle) final;
    VulkanAdapter *getAdapter(const Handle<Adapter_t> &handle) final { return m_adapters.get(handle); }

    // virtual Handle<Shader> createShader(ShaderDescription desc) = 0;
    Handle<BindGroup> createBindGroup(BindGroupDescription desc) final;
    // virtual Handle<Texture> createTexture(TextureDescription desc) = 0;
    // virtual Handle<Buffer> createBuffer(BufferDescription desc) = 0;

    // virtual void deleteShader(Handle<Shader> handle) = 0;
    void deleteBindGroup(Handle<BindGroup> handle) final;
    // virtual void deleteTexture(Handle<Texture> handle) = 0;
    // virtual void deleteBuffer(Handle<Buffer> handle) = 0;

private:
    Pool<VulkanInstance, Instance_t> m_instances{ 1 };
    Pool<VulkanAdapter, Adapter_t> m_adapters{ 1 };
};

} // namespace ToyRenderer
