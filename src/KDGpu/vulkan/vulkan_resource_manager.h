/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/resource_manager.h>

#include <KDGpu/pool.h>

#include <KDGpu/vulkan/vulkan_adapter.h>
#include <KDGpu/vulkan/vulkan_bind_group.h>
#include <KDGpu/vulkan/vulkan_bind_group_layout.h>
#include <KDGpu/vulkan/vulkan_buffer.h>
#include <KDGpu/vulkan/vulkan_command_buffer.h>
#include <KDGpu/vulkan/vulkan_command_recorder.h>
#include <KDGpu/vulkan/vulkan_compute_pipeline.h>
#include <KDGpu/vulkan/vulkan_compute_pass_command_recorder.h>
#include <KDGpu/vulkan/vulkan_device.h>
#include <KDGpu/vulkan/vulkan_fence.h>
#include <KDGpu/vulkan/vulkan_framebuffer.h>
#include <KDGpu/vulkan/vulkan_gpu_semaphore.h>
#include <KDGpu/vulkan/vulkan_graphics_pipeline.h>
#include <KDGpu/vulkan/vulkan_instance.h>
#include <KDGpu/vulkan/vulkan_pipeline_layout.h>
#include <KDGpu/vulkan/vulkan_queue.h>
#include <KDGpu/vulkan/vulkan_render_pass.h>
#include <KDGpu/vulkan/vulkan_render_pass_command_recorder.h>
#include <KDGpu/vulkan/vulkan_sampler.h>
#include <KDGpu/vulkan/vulkan_shader_module.h>
#include <KDGpu/vulkan/vulkan_swapchain.h>
#include <KDGpu/vulkan/vulkan_surface.h>
#include <KDGpu/vulkan/vulkan_texture.h>
#include <KDGpu/vulkan/vulkan_texture_view.h>
#include <KDGpu/vulkan/vulkan_timestamp_query_recorder.h>

#include <KDGpu/kdgpu_export.h>

#include <vulkan/vulkan.h>

namespace KDGpu {

/**
 * @brief VulkanResourceManager
 * \ingroup vulkan
 *
 */

struct RenderTargetOptions;
struct DepthStencilOptions;

class KDGPU_EXPORT VulkanResourceManager final : public ResourceManager
{
public:
    VulkanResourceManager();
    ~VulkanResourceManager() final;

    Handle<Instance_t> createInstance(const InstanceOptions &options) final;
    Handle<Instance_t> createInstanceFromExistingVkInstance(VkInstance vkInstance);
    void deleteInstance(const Handle<Instance_t> &handle) final;
    VulkanInstance *getInstance(const Handle<Instance_t> &handle) const final;
    std::vector<Extension> getInstanceExtensions() const;

    Handle<Adapter_t> insertAdapter(const VulkanAdapter &vulkanAdapter);
    void removeAdapter(const Handle<Adapter_t> &handle) final;
    VulkanAdapter *getAdapter(const Handle<Adapter_t> &handle) const final;

    Handle<Device_t> createDevice(const Handle<Adapter_t> &adapterHandle, const DeviceOptions &options, std::vector<QueueRequest> &queueRequests) final;
    Handle<Device_t> createDeviceFromExistingVkDevice(const Handle<Adapter_t> &adapterHandle, VkDevice vkDevice);
    void deleteDevice(const Handle<Device_t> &handle) final;
    VulkanDevice *getDevice(const Handle<Device_t> &handle) const final;

    Handle<Queue_t> insertQueue(const VulkanQueue &vulkanQueue);
    void removeQueue(const Handle<Queue_t> &handle) final;
    VulkanQueue *getQueue(const Handle<Queue_t> &handle) const final;

    Handle<Swapchain_t> createSwapchain(const Handle<Device_t> &deviceHandle, const SwapchainOptions &options) final;
    void deleteSwapchain(const Handle<Swapchain_t> &handle) final;
    VulkanSwapchain *getSwapchain(const Handle<Swapchain_t> &handle) const final;

    Handle<Surface_t> insertSurface(const VulkanSurface &surface);
    void deleteSurface(const Handle<Surface_t> &handle) final;
    VulkanSurface *getSurface(const Handle<Surface_t> &handle) const final;

    // For swapchain-owned images
    Handle<Texture_t> insertTexture(const VulkanTexture &texture);
    void removeTexture(const Handle<Texture_t> &handle);

    // For user-created textures
    Handle<Texture_t> createTexture(const Handle<Device_t> &deviceHandle, const TextureOptions &options) final;
    void deleteTexture(const Handle<Texture_t> &handle) final;
    VulkanTexture *getTexture(const Handle<Texture_t> &handle) const final;

    Handle<TextureView_t> createTextureView(const Handle<Device_t> &deviceHandle, const Handle<Texture_t> &textureHandle, const TextureViewOptions &options) final;
    void deleteTextureView(const Handle<TextureView_t> &handle) final;
    VulkanTextureView *getTextureView(const Handle<TextureView_t> &handle) const final;

    Handle<Buffer_t> createBuffer(const Handle<Device_t> &deviceHandle, const BufferOptions &options, const void *initialData) final;
    void deleteBuffer(const Handle<Buffer_t> &handle) final;
    VulkanBuffer *getBuffer(const Handle<Buffer_t> &handle) const final;

    Handle<ShaderModule_t> createShaderModule(const Handle<Device_t> &deviceHandle, const std::vector<uint32_t> &code) final;
    void deleteShaderModule(const Handle<ShaderModule_t> &handle) final;
    VulkanShaderModule *getShaderModule(const Handle<ShaderModule_t> &handle) const final;

    Handle<PipelineLayout_t> createPipelineLayout(const Handle<Device_t> &deviceHandle, const PipelineLayoutOptions &options) final;
    void deletePipelineLayout(const Handle<PipelineLayout_t> &handle) final;
    VulkanPipelineLayout *getPipelineLayout(const Handle<PipelineLayout_t> &handle) const final;

    Handle<GraphicsPipeline_t> createGraphicsPipeline(const Handle<Device_t> &deviceHandle, const GraphicsPipelineOptions &options) final;
    void deleteGraphicsPipeline(const Handle<GraphicsPipeline_t> &handle) final;
    VulkanGraphicsPipeline *getGraphicsPipeline(const Handle<GraphicsPipeline_t> &handle) const final;

    Handle<ComputePipeline_t> createComputePipeline(const Handle<Device_t> &deviceHandle, const ComputePipelineOptions &options) final;
    void deleteComputePipeline(const Handle<ComputePipeline_t> &handle) final;
    VulkanComputePipeline *getComputePipeline(const Handle<ComputePipeline_t> &handle) const final;

    Handle<GpuSemaphore_t> createGpuSemaphore(const Handle<Device_t> &deviceHandle, const GpuSemaphoreOptions &options) final;
    void deleteGpuSemaphore(const Handle<GpuSemaphore_t> &handle) final;
    VulkanGpuSemaphore *getGpuSemaphore(const Handle<GpuSemaphore_t> &handle) const final;

    Handle<CommandRecorder_t> createCommandRecorder(const Handle<Device_t> &deviceHandle, const CommandRecorderOptions &options) final;
    void deleteCommandRecorder(const Handle<CommandRecorder_t> &handle) final;
    VulkanCommandRecorder *getCommandRecorder(const Handle<CommandRecorder_t> &handle) const final;

    Handle<RenderPassCommandRecorder_t> createRenderPassCommandRecorder(const Handle<Device_t> &deviceHandle,
                                                                        const Handle<CommandRecorder_t> &commandRecorderHandle,
                                                                        const RenderPassCommandRecorderOptions &options) final;
    void deleteRenderPassCommandRecorder(const Handle<RenderPassCommandRecorder_t> &handle) final;
    VulkanRenderPassCommandRecorder *getRenderPassCommandRecorder(const Handle<RenderPassCommandRecorder_t> &handle) const final;

    Handle<ComputePassCommandRecorder_t> createComputePassCommandRecorder(const Handle<Device_t> &deviceHandle,
                                                                          const Handle<CommandRecorder_t> &commandRecorderHandle,
                                                                          const ComputePassCommandRecorderOptions &options) final;
    void deleteComputePassCommandRecorder(const Handle<ComputePassCommandRecorder_t> &handle) final;
    VulkanComputePassCommandRecorder *getComputePassCommandRecorder(const Handle<ComputePassCommandRecorder_t> &handle) const final;

    Handle<TimestampQueryRecorder_t> createTimestampQueryRecorder(const Handle<Device_t> &deviceHandle,
                                                                  const Handle<CommandRecorder_t> &commandRecorderHandle,
                                                                  const TimestampQueryRecorderOptions &options) final;
    void deleteTimestampQueryRecorder(const Handle<TimestampQueryRecorder_t> &handle) final;
    VulkanTimestampQueryRecorder *getTimestampQueryRecorder(const Handle<TimestampQueryRecorder_t> &handle) const final;

    // Command buffers are not created by the api. It is up to the concrete subclasses to insert the command buffers
    // by whatever mechanism they wish. They also do not need to be destroyed as they are cleaned up by the owning
    // command pool (command recorder).
    // Yet we add the usual create/destroy functions to make it more consistent
    Handle<CommandBuffer_t> createCommandBuffer(const Handle<Device_t> &deviceHandle,
                                                const QueueDescription &queueDescription,
                                                CommandBufferLevel commandLevel) final;
    void deleteCommandBuffer(const Handle<CommandBuffer_t> &handle) final;
    VulkanCommandBuffer *getCommandBuffer(const Handle<CommandBuffer_t> &handle) const final;

    Handle<BindGroup_t> createBindGroup(const Handle<Device_t> &deviceHandle, const BindGroupOptions &options) final;
    void deleteBindGroup(const Handle<BindGroup_t> &handle) final;
    VulkanBindGroup *getBindGroup(const Handle<BindGroup_t> &handle) const final;

    Handle<BindGroupLayout_t> createBindGroupLayout(const Handle<Device_t> &deviceHandle, const BindGroupLayoutOptions &options) final;
    void deleteBindGroupLayout(const Handle<BindGroupLayout_t> &handle) final;
    VulkanBindGroupLayout *getBindGroupLayout(const Handle<BindGroupLayout_t> &handle) const final;

    Handle<Sampler_t> createSampler(const Handle<Device_t> &deviceHandle, const SamplerOptions &options) final;
    void deleteSampler(const Handle<Sampler_t> &handle) final;
    VulkanSampler *getSampler(const Handle<Sampler_t> &handle) const final;

    Handle<Fence_t> createFence(const Handle<Device_t> &deviceHandle, const FenceOptions &options) final;
    void deleteFence(const Handle<Fence_t> &handle) final;
    VulkanFence *getFence(const Handle<Fence_t> &handle) const final;

private:
    void fillColorAttachmnents(std::vector<VkAttachmentReference2> &colorAttachmentRefs,
                               std::vector<VkAttachmentReference2> &colorResolveAttachmentRefs,
                               std::vector<VkAttachmentDescription2> &attachments,
                               const std::vector<ColorAttachment> &colorAttachments,
                               SampleCountFlagBits samples);

    void fillColorAttachmnents(std::vector<VkAttachmentReference2> &colorAttachmentRefs,
                               std::vector<VkAttachmentReference2> &colorResolveAttachmentRefs,
                               std::vector<VkAttachmentDescription2> &attachments,
                               const std::vector<RenderTargetOptions> &colorAttachments,
                               SampleCountFlagBits samples);

    std::pair<bool, bool> fillDepthAttachments(VkAttachmentReference2 &depthStencilAttachmentRef,
                                               VkAttachmentReference2 &depthStencilResolveAttachmentRef,
                                               std::vector<VkAttachmentDescription2> &attachments,
                                               VkSubpassDescriptionDepthStencilResolve &depthResolve,
                                               const DepthStencilAttachment &depthStencilAttachment,
                                               SampleCountFlagBits samples);

    std::pair<bool, bool> fillDepthAttachments(VkAttachmentReference2 &depthStencilAttachmentRef,
                                               VkAttachmentReference2 &depthStencilResolveAttachmentRef,
                                               std::vector<VkAttachmentDescription2> &attachments,
                                               VkSubpassDescriptionDepthStencilResolve &depthResolve,
                                               const DepthStencilOptions &depthStencilAttachment,
                                               SampleCountFlagBits samples);

    template<typename ColorAtt, typename DepthAtt>
    Handle<RenderPass_t> createRenderPass(const Handle<Device_t> &deviceHandle,
                                          const std::vector<ColorAtt> &colorAttachments,
                                          const DepthAtt &depthAttachment,
                                          SampleCountFlagBits samples,
                                          uint32_t viewCount);
    Handle<Framebuffer_t> createFramebuffer(const Handle<Device_t> &deviceHandle, const RenderPassCommandRecorderOptions &options, const VulkanFramebufferKey &frameBufferKey);

    void setObjectName(VulkanDevice *device, const VkObjectType type, const uint64_t handle, const std::string_view name);

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
    Pool<VulkanBindGroupLayout, BindGroupLayout_t> m_bindGroupLayouts{ 128 };
    Pool<VulkanBindGroup, BindGroup_t> m_bindGroups{ 128 };
    Pool<VulkanGraphicsPipeline, GraphicsPipeline_t> m_graphicsPipelines{ 64 };
    Pool<VulkanComputePipeline, ComputePipeline_t> m_computePipelines{ 64 };
    Pool<VulkanGpuSemaphore, GpuSemaphore_t> m_gpuSemaphores{ 32 };
    Pool<VulkanCommandRecorder, CommandRecorder_t> m_commandRecorders{ 32 };
    Pool<VulkanRenderPassCommandRecorder, RenderPassCommandRecorder_t> m_renderPassCommandRecorders{ 32 };
    Pool<VulkanComputePassCommandRecorder, ComputePassCommandRecorder_t> m_computePassCommandRecorders{ 32 };
    Pool<VulkanCommandBuffer, CommandBuffer_t> m_commandBuffers{ 128 };
    Pool<VulkanRenderPass, RenderPass_t> m_renderPasses{ 16 };
    Pool<VulkanFramebuffer, Framebuffer_t> m_framebuffers{ 16 };
    Pool<VulkanSampler, Sampler_t> m_samplers{ 16 };
    Pool<VulkanFence, Fence_t> m_fences{ 16 };
    Pool<VulkanTimestampQueryRecorder, TimestampQueryRecorder_t> m_timestampQueryRecorders{ 4 };

    struct TimestampQueryBucket {
        uint32_t start;
        uint32_t count;
    };
    std::vector<TimestampQueryBucket> m_timestampQueryBuckets;
};

} // namespace KDGpu
