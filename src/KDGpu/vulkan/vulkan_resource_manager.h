/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/vulkan/vulkan_adapter.h>
#include <KDGpu/vulkan/vulkan_bind_group.h>
#include <KDGpu/vulkan/vulkan_bind_group_layout.h>
#include <KDGpu/vulkan/vulkan_bind_group_pool.h>
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
#include <KDGpu/vulkan/vulkan_acceleration_structure.h>
#include <KDGpu/vulkan/vulkan_raytracing_pipeline.h>
#include <KDGpu/vulkan/vulkan_raytracing_pass_command_recorder.h>
#include <KDGpu/vulkan/vulkan_ycbcr_conversion.h>

#include <KDGpu/instance.h>
#include <KDGpu/pool.h>
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
struct BindGroupPoolOptions;
struct ShaderStage;

class KDGPU_EXPORT VulkanResourceManager
{
public:
    VulkanResourceManager();
    ~VulkanResourceManager();

    Handle<Instance_t> createInstance(const InstanceOptions &options);
    Handle<Instance_t> createInstanceFromExistingVkInstance(VkInstance vkInstance);
    void deleteInstance(const Handle<Instance_t> &handle);
    [[nodiscard]] VulkanInstance *getInstance(const Handle<Instance_t> &handle) const;
    [[nodiscard]] static std::vector<Extension> getInstanceExtensions();

    Handle<Adapter_t> insertAdapter(const VulkanAdapter &physicalDevice);
    void removeAdapter(const Handle<Adapter_t> &handle);
    [[nodiscard]] VulkanAdapter *getAdapter(const Handle<Adapter_t> &handle) const;

    Handle<Device_t> createDevice(const Handle<Adapter_t> &adapterHandle, const DeviceOptions &options, std::vector<QueueRequest> &queueRequests);
    Handle<Device_t> createDeviceFromExistingVkDevice(const Handle<Adapter_t> &adapterHandle, VkDevice vkDevice);
    void deleteDevice(const Handle<Device_t> &handle);
    [[nodiscard]] VulkanDevice *getDevice(const Handle<Device_t> &handle) const;

    Handle<Queue_t> insertQueue(const VulkanQueue &vulkanQueue);
    void removeQueue(const Handle<Queue_t> &handle);
    [[nodiscard]] VulkanQueue *getQueue(const Handle<Queue_t> &handle) const;

    Handle<Swapchain_t> createSwapchain(const Handle<Device_t> &deviceHandle, const SwapchainOptions &options);
    void deleteSwapchain(const Handle<Swapchain_t> &handle);
    [[nodiscard]] VulkanSwapchain *getSwapchain(const Handle<Swapchain_t> &handle) const;

    Handle<Surface_t> insertSurface(const VulkanSurface &surface);
    void deleteSurface(const Handle<Surface_t> &handle);
    [[nodiscard]] VulkanSurface *getSurface(const Handle<Surface_t> &handle) const;

    // For swapchain-owned images
    Handle<Texture_t> insertTexture(const VulkanTexture &texture);
    void removeTexture(const Handle<Texture_t> &handle);

    // For user-created textures
    Handle<Texture_t> createTexture(const Handle<Device_t> &deviceHandle, const TextureOptions &options);
    void deleteTexture(const Handle<Texture_t> &handle);
    [[nodiscard]] VulkanTexture *getTexture(const Handle<Texture_t> &handle) const;

    Handle<TextureView_t> createTextureView(const Handle<Device_t> &deviceHandle, const Handle<Texture_t> &textureHandle, const TextureViewOptions &options);
    void deleteTextureView(const Handle<TextureView_t> &handle);
    [[nodiscard]] VulkanTextureView *getTextureView(const Handle<TextureView_t> &handle) const;

    Handle<Buffer_t> createBuffer(const Handle<Device_t> &deviceHandle, const BufferOptions &options, const void *initialData);
    void deleteBuffer(const Handle<Buffer_t> &handle);
    [[nodiscard]] VulkanBuffer *getBuffer(const Handle<Buffer_t> &handle) const;

    Handle<ShaderModule_t> createShaderModule(const Handle<Device_t> &deviceHandle, const std::vector<uint32_t> &code);
    void deleteShaderModule(const Handle<ShaderModule_t> &handle);
    [[nodiscard]] VulkanShaderModule *getShaderModule(const Handle<ShaderModule_t> &handle) const;

    Handle<RenderPass_t> createRenderPass(const Handle<Device_t> &deviceHandle, const RenderPassOptions &options);
    void deleteRenderPass(const Handle<RenderPass_t> &handle);
    [[nodiscard]] VulkanRenderPass *getRenderPass(const Handle<RenderPass_t> &handle);

    Handle<PipelineLayout_t> createPipelineLayout(const Handle<Device_t> &deviceHandle, const PipelineLayoutOptions &options);
    void deletePipelineLayout(const Handle<PipelineLayout_t> &handle);
    [[nodiscard]] VulkanPipelineLayout *getPipelineLayout(const Handle<PipelineLayout_t> &handle) const;

    Handle<GraphicsPipeline_t> createGraphicsPipeline(const Handle<Device_t> &deviceHandle, const GraphicsPipelineOptions &options);
    void deleteGraphicsPipeline(const Handle<GraphicsPipeline_t> &handle);
    [[nodiscard]] VulkanGraphicsPipeline *getGraphicsPipeline(const Handle<GraphicsPipeline_t> &handle) const;

    Handle<ComputePipeline_t> createComputePipeline(const Handle<Device_t> &deviceHandle, const ComputePipelineOptions &options);
    void deleteComputePipeline(const Handle<ComputePipeline_t> &handle);
    [[nodiscard]] VulkanComputePipeline *getComputePipeline(const Handle<ComputePipeline_t> &handle) const;

    Handle<RayTracingPipeline_t> createRayTracingPipeline(const Handle<Device_t> &deviceHandle, const RayTracingPipelineOptions &options);
    void deleteRayTracingPipeline(const Handle<RayTracingPipeline_t> &handle);
    [[nodiscard]] VulkanRayTracingPipeline *getRayTracingPipeline(const Handle<RayTracingPipeline_t> &handle) const;

    Handle<GpuSemaphore_t> createGpuSemaphore(const Handle<Device_t> &deviceHandle, const GpuSemaphoreOptions &options);
    void deleteGpuSemaphore(const Handle<GpuSemaphore_t> &handle);
    [[nodiscard]] VulkanGpuSemaphore *getGpuSemaphore(const Handle<GpuSemaphore_t> &handle) const;

    Handle<CommandRecorder_t> createCommandRecorder(const Handle<Device_t> &deviceHandle, const CommandRecorderOptions &options);
    void deleteCommandRecorder(const Handle<CommandRecorder_t> &handle);
    [[nodiscard]] VulkanCommandRecorder *getCommandRecorder(const Handle<CommandRecorder_t> &handle) const;

    Handle<RenderPassCommandRecorder_t> createRenderPassCommandRecorder(const Handle<Device_t> &deviceHandle,
                                                                        const Handle<CommandRecorder_t> &commandRecorderHandle,
                                                                        const RenderPassCommandRecorderOptions &options);
    Handle<RenderPassCommandRecorder_t> createRenderPassCommandRecorder(const Handle<Device_t> &deviceHandle,
                                                                        const Handle<CommandRecorder_t> &commandRecorderHandle,
                                                                        const RenderPassCommandRecorderWithRenderPassOptions &options);
    Handle<RenderPassCommandRecorder_t> createRenderPassCommandRecorder(const Handle<Device_t> &deviceHandle,
                                                                        const Handle<CommandRecorder_t> &commandRecorderHandle,
                                                                        const RenderPassCommandRecorderWithDynamicRenderingOptions &options);
    void deleteRenderPassCommandRecorder(const Handle<RenderPassCommandRecorder_t> &handle);
    [[nodiscard]] VulkanRenderPassCommandRecorder *getRenderPassCommandRecorder(const Handle<RenderPassCommandRecorder_t> &handle) const;

    Handle<ComputePassCommandRecorder_t> createComputePassCommandRecorder(const Handle<Device_t> &deviceHandle,
                                                                          const Handle<CommandRecorder_t> &commandRecorderHandle,
                                                                          const ComputePassCommandRecorderOptions &options);
    void deleteComputePassCommandRecorder(const Handle<ComputePassCommandRecorder_t> &handle);
    [[nodiscard]] VulkanComputePassCommandRecorder *getComputePassCommandRecorder(const Handle<ComputePassCommandRecorder_t> &handle) const;

    void deleteRayTracingPassCommandRecorder(const Handle<RayTracingPassCommandRecorder_t> &handle);
    [[nodiscard]] VulkanRayTracingPassCommandRecorder *getRayTracingPassCommandRecorder(const Handle<RayTracingPassCommandRecorder_t> &handle) const;

    Handle<RayTracingPassCommandRecorder_t> createRayTracingPassCommandRecorder(const Handle<Device_t> &deviceHandle,
                                                                                const Handle<CommandRecorder_t> &commandRecorderHandle,
                                                                                const RayTracingPassCommandRecorderOptions &options);

    Handle<TimestampQueryRecorder_t> createTimestampQueryRecorder(const Handle<Device_t> &deviceHandle,
                                                                  const Handle<CommandRecorder_t> &commandRecorderHandle,
                                                                  const TimestampQueryRecorderOptions &options);
    void deleteTimestampQueryRecorder(const Handle<TimestampQueryRecorder_t> &handle);
    [[nodiscard]] VulkanTimestampQueryRecorder *getTimestampQueryRecorder(const Handle<TimestampQueryRecorder_t> &handle) const;

    // Command buffers are not created by the api. It is up to the concrete subclasses to insert the command buffers
    // by whatever mechanism they wish. They also do not need to be destroyed as they are cleaned up by the owning
    // command pool (command recorder).
    // Yet we add the usual create/destroy functions to make it more consistent
    Handle<CommandBuffer_t> createCommandBuffer(const Handle<Device_t> &deviceHandle,
                                                const QueueDescription &queueDescription,
                                                CommandBufferLevel commandLevel);
    void deleteCommandBuffer(const Handle<CommandBuffer_t> &handle);
    [[nodiscard]] VulkanCommandBuffer *getCommandBuffer(const Handle<CommandBuffer_t> &handle) const;

    Handle<BindGroupPool_t> createBindGroupPool(const Handle<Device_t> &deviceHandle, const BindGroupPoolOptions &options);
    void deleteBindGroupPool(const Handle<BindGroupPool_t> &handle);
    [[nodiscard]] VulkanBindGroupPool *getBindGroupPool(const Handle<BindGroupPool_t> &handle) const;

    Handle<BindGroup_t> createBindGroup(const Handle<Device_t> &deviceHandle, const BindGroupOptions &options);
    void deleteBindGroup(const Handle<BindGroup_t> &handle);
    [[nodiscard]] VulkanBindGroup *getBindGroup(const Handle<BindGroup_t> &handle) const;

    Handle<BindGroupLayout_t> createBindGroupLayout(const Handle<Device_t> &deviceHandle, const BindGroupLayoutOptions &options);
    void deleteBindGroupLayout(const Handle<BindGroupLayout_t> &handle);
    [[nodiscard]] VulkanBindGroupLayout *getBindGroupLayout(const Handle<BindGroupLayout_t> &handle) const;

    Handle<Sampler_t> createSampler(const Handle<Device_t> &deviceHandle, const SamplerOptions &options);
    void deleteSampler(const Handle<Sampler_t> &handle);
    [[nodiscard]] VulkanSampler *getSampler(const Handle<Sampler_t> &handle) const;

    Handle<Fence_t> createFence(const Handle<Device_t> &deviceHandle, const FenceOptions &options);
    void deleteFence(const Handle<Fence_t> &handle);
    [[nodiscard]] VulkanFence *getFence(const Handle<Fence_t> &handle) const;
    Handle<AccelerationStructure_t> createAccelerationStructure(const Handle<Device_t> &deviceHandle, const AccelerationStructureOptions &options);
    void deleteAccelerationStructure(const Handle<AccelerationStructure_t> &handle);
    VulkanAccelerationStructure *getAccelerationStructure(const Handle<AccelerationStructure_t> &handle) const;

    Handle<YCbCrConversion_t> createYCbCrConversion(const Handle<Device_t> &deviceHandle, const YCbCrConversionOptions &options);
    void deleteYCbCrConversion(const Handle<YCbCrConversion_t> &handle);
    [[nodiscard]] VulkanYCbCrConversion *getYCbCrConversion(const Handle<YCbCrConversion_t> &handle) const;

    [[nodiscard]] std::string getMemoryStats(const Handle<Device_t> &device) const;

    [[nodiscard]] KDGpu::Format formatFromTextureView(const Handle<TextureView_t> &viewHandle) const;

private:
    [[nodiscard]] SubpassDescription fillAttachmentDescriptionAndCreateSubpassDescription(std::vector<AttachmentDescription> &attachmentDescriptions,
                                                                                          const std::vector<ColorAttachment> &colorAttachments,
                                                                                          const DepthStencilAttachment &depthAttachment,
                                                                                          SampleCountFlagBits samples) const;

    [[nodiscard]] static SubpassDescription fillAttachmentDescriptionAndCreateSubpassDescription(std::vector<AttachmentDescription> &attachmentDescriptions,
                                                                                                 const std::vector<RenderTargetOptions> &colorAttachments,
                                                                                                 const DepthStencilOptions &depthStencilAttachment,
                                                                                                 SampleCountFlagBits samples);
    struct ShaderStagesInfo {
        std::vector<VkPipelineShaderStageCreateInfo> shaderInfos;
        std::vector<VkSpecializationInfo> shaderSpecializationInfos;
        std::vector<std::vector<VkSpecializationMapEntry>> shaderSpecializationMapEntries;
        std::vector<std::vector<uint8_t>> shaderSpecializationRawData;
    };

    bool fillShaderStageInfos(const std::vector<ShaderStage> &stages,
                              ShaderStagesInfo &shaderStagesInfo) const;

    // For RenderPassCommandRecorder implicit RenderPass creation
    Handle<RenderPass_t> createImplicitRenderPass(const Handle<Device_t> &deviceHandle,
                                                  const std::vector<ColorAttachment> &colorAttachments,
                                                  const DepthStencilAttachment &depthStencilAttachment,
                                                  SampleCountFlagBits samples,
                                                  uint32_t viewCount);

    // For GraphicsPipeline implicit RenderPass creation
    Handle<RenderPass_t> createImplicitRenderPass(const Handle<Device_t> &deviceHandle,
                                                  const std::vector<RenderTargetOptions> &colorAttachments,
                                                  const DepthStencilOptions &depthStencilAttachment,
                                                  SampleCountFlagBits samples,
                                                  uint32_t viewCount);

    Handle<Framebuffer_t> createFramebuffer(const Handle<Device_t> &deviceHandle,
                                            const VulkanFramebufferKey &frameBufferKey);
    void deleteFramebuffer(const Handle<Framebuffer_t> &handle);

    static void setObjectName(VulkanDevice *device, VkObjectType type, uint64_t handle, std::string_view name);

    [[nodiscard]] static std::vector<std::string> getAvailableLayers();

    [[nodiscard]] static MemoryHandle retrieveExternalMemoryHandle(VulkanInstance *instance,
                                                                   VulkanDevice *vulkanDevice,
                                                                   const VmaAllocationInfo &allocationInfo,
                                                                   ExternalMemoryHandleTypeFlags handleType);

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
    Pool<VulkanBindGroupPool, BindGroupPool_t> m_bindGroupPools{ 4 };
    Pool<VulkanGraphicsPipeline, GraphicsPipeline_t> m_graphicsPipelines{ 64 };
    Pool<VulkanComputePipeline, ComputePipeline_t> m_computePipelines{ 64 };
    Pool<VulkanRayTracingPipeline, RayTracingPipeline_t> m_rayTracingPipelines{ 64 };
    Pool<VulkanGpuSemaphore, GpuSemaphore_t> m_gpuSemaphores{ 32 };
    Pool<VulkanCommandRecorder, CommandRecorder_t> m_commandRecorders{ 32 };
    Pool<VulkanRenderPassCommandRecorder, RenderPassCommandRecorder_t> m_renderPassCommandRecorders{ 32 };
    Pool<VulkanComputePassCommandRecorder, ComputePassCommandRecorder_t> m_computePassCommandRecorders{ 32 };
    Pool<VulkanRayTracingPassCommandRecorder, RayTracingPassCommandRecorder_t> m_rayTracingPassCommandRecorders{ 32 };
    Pool<VulkanCommandBuffer, CommandBuffer_t> m_commandBuffers{ 128 };
    Pool<VulkanRenderPass, RenderPass_t> m_renderPasses{ 16 };
    Pool<VulkanFramebuffer, Framebuffer_t> m_framebuffers{ 16 };
    Pool<VulkanSampler, Sampler_t> m_samplers{ 16 };
    Pool<VulkanFence, Fence_t> m_fences{ 16 };
    Pool<VulkanTimestampQueryRecorder, TimestampQueryRecorder_t> m_timestampQueryRecorders{ 4 };
    Pool<VulkanAccelerationStructure, AccelerationStructure_t> m_accelerationStructures{ 32 };
    Pool<VulkanYCbCrConversion, YCbCrConversion_t> m_yCbCrConversions{ 16 };
    struct TimestampQueryBucket {
        uint32_t start;
        uint32_t count;
    };
    std::vector<TimestampQueryBucket> m_timestampQueryBuckets;
};

} // namespace KDGpu
