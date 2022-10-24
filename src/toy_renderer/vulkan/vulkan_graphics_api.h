#pragma once

#include <toy_renderer/graphics_api.h>
#include <toy_renderer/toy_renderer_export.h>
#include <toy_renderer/vulkan/vulkan_resource_manager.h>

#include <memory>

namespace ToyRenderer {

class TOY_RENDERER_EXPORT VulkanGraphicsApi : public GraphicsApi
{
public:
    VulkanGraphicsApi();
    ~VulkanGraphicsApi() final;

    std::vector<Handle<Adapter_t>> queryAdapters(const Handle<Instance_t> &instanceHandle) final;
    AdapterProperties queryAdapterProperties(const Handle<Adapter_t> &adapterHandle) final;

private:
    std::unique_ptr<VulkanResourceManager> m_vulkanResourceManager;
};

} // namespace ToyRenderer
