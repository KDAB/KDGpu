#pragma once

#include <toy_renderer/resource_manager.h>

#include <toy_renderer/pool.h>

#include <toy_renderer/vulkan/vulkan_adapter.h>
#include <toy_renderer/vulkan/vulkan_buffer.h>
#include <toy_renderer/vulkan/vulkan_device.h>
#include <toy_renderer/vulkan/vulkan_graphics_pipeline.h>
#include <toy_renderer/vulkan/vulkan_instance.h>
#include <toy_renderer/vulkan/vulkan_pipeline_layout.h>
#include <toy_renderer/vulkan/vulkan_queue.h>
#include <toy_renderer/vulkan/vulkan_shader_module.h>
#include <toy_renderer/vulkan/vulkan_swapchain.h>
#include <toy_renderer/vulkan/vulkan_surface.h>
#include <toy_renderer/vulkan/vulkan_texture.h>
#include <toy_renderer/vulkan/vulkan_texture_view.h>

#include <toy_renderer/toy_renderer_export.h>

#include <vulkan/vulkan.h>

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

    Handle<Device_t> createDevice(const Handle<Adapter_t> &adapterHandle, const DeviceOptions &options, std::vector<QueueRequest> &queueRequests) final;
    void deleteDevice(Handle<Device_t> handle) final;
    VulkanDevice *getDevice(const Handle<Device_t> &handle) final { return m_devices.get(handle); }

    Handle<Queue_t> insertQueue(const VulkanQueue &vulkanQueue);
    void removeQueue(Handle<Queue_t> handle) final;
    VulkanQueue *getQueue(const Handle<Queue_t> &handle) final { return m_queues.get(handle); }

    Handle<Swapchain_t> createSwapchain(const Handle<Device_t> &deviceHandle, const SwapchainOptions &options) final;
    void deleteSwapchain(Handle<Swapchain_t> handle) final;
    VulkanSwapchain *getSwapchain(const Handle<Swapchain_t> &handle) final { return m_swapchains.get(handle); }

    Handle<Surface_t> insertSurface(const VulkanSurface &surface);
    void deleteSurface(Handle<Surface_t> handle) final;
    VulkanSurface *getSurface(const Handle<Surface_t> &handle) final { return m_surfaces.get(handle); }

    // For swapchain-owned images
    Handle<Texture_t> insertTexture(const VulkanTexture &texture);
    void removeTexture(Handle<Texture_t> handle);

    // For user-created textures
    Handle<Texture_t> createTexture(const Handle<Device_t> deviceHandle, const TextureOptions &options) final;
    void deleteTexture(Handle<Texture_t> handle) final;

    VulkanTexture *getTexture(const Handle<Texture_t> &handle) final { return m_textures.get(handle); }

    Handle<TextureView_t> createTextureView(const Handle<Device_t> &deviceHandle, const Handle<Texture_t> &textureHandle, const TextureViewOptions &options) final;
    void deleteTextureView(Handle<TextureView_t> handle) final;
    VulkanTextureView *getTextureView(const Handle<TextureView_t> &handle) final { return m_textureViews.get(handle); }

    Handle<Buffer_t> createBuffer(const Handle<Device_t> deviceHandle, const BufferOptions &options, void *initialData) final;
    void deleteBuffer(Handle<Buffer_t> handle) final;
    VulkanBuffer *getBuffer(const Handle<Buffer_t> &handle) final { return m_buffers.get(handle); }

    Handle<ShaderModule_t> createShaderModule(const Handle<Device_t> deviceHandle, const std::vector<uint32_t> &code) final;
    void deleteShaderModule(Handle<ShaderModule_t> handle) final;
    VulkanShaderModule *getShaderModule(const Handle<ShaderModule_t> &handle) final { return m_shaderModules.get(handle); }

    Handle<PipelineLayout_t> createPipelineLayout(const Handle<Device_t> &deviceHandle, const PipelineLayoutOptions &options) final;
    void deletePipelineLayout(Handle<PipelineLayout_t> handle) final;
    VulkanPipelineLayout *getPipelineLayout(const Handle<PipelineLayout_t> &handle) final { return m_pipelineLayouts.get(handle); }

    Handle<GraphicsPipeline_t> createGraphicsPipeline(const Handle<Device_t> &deviceHandle, const GraphicsPipelineOptions &options) final;
    void deleteGraphicsPipeline(Handle<GraphicsPipeline_t> handle) final;
    VulkanGraphicsPipeline *getGraphicsPipeline(const Handle<GraphicsPipeline_t> &handle) final { return m_graphicsPipelines.get(handle); }

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
    Pool<VulkanDevice, Device_t> m_devices{ 1 };
    Pool<VulkanQueue, Queue_t> m_queues{ 4 };
    Pool<VulkanSurface, Surface_t> m_surfaces{ 1 };
    Pool<VulkanSwapchain, Swapchain_t> m_swapchains{ 1 };
    Pool<VulkanTexture, Texture_t> m_textures{ 128 };
    Pool<VulkanTextureView, TextureView_t> m_textureViews{ 128 };
    Pool<VulkanBuffer, Buffer_t> m_buffers{ 128 };
    Pool<VulkanShaderModule, ShaderModule_t> m_shaderModules{ 64 };
    Pool<VulkanPipelineLayout, PipelineLayout_t> m_pipelineLayouts{ 64 };
    Pool<VulkanGraphicsPipeline, GraphicsPipeline_t> m_graphicsPipelines{ 64 };
};

} // namespace ToyRenderer
