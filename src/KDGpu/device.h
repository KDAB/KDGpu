/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/bind_group.h>
#include <KDGpu/bind_group_layout.h>
#include <KDGpu/bind_group_pool.h>
#include <KDGpu/buffer.h>
#include <KDGpu/command_recorder.h>
#include <KDGpu/compute_pipeline.h>
#include <KDGpu/fence.h>
#include <KDGpu/gpu_semaphore.h>
#include <KDGpu/timeline_semaphore.h>
#include <KDGpu/graphics_pipeline.h>
#include <KDGpu/handle.h>
#include <KDGpu/pipeline_layout.h>
#include <KDGpu/pipeline_layout_options.h>
#include <KDGpu/queue.h>
#include <KDGpu/sampler.h>
#include <KDGpu/sampler_options.h>
#include <KDGpu/shader_module.h>
#include <KDGpu/swapchain.h>
#include <KDGpu/acceleration_structure.h>
#include <KDGpu/acceleration_structure_options.h>
#include <KDGpu/raytracing_pipeline.h>
#include <KDGpu/render_pass.h>
#include <KDGpu/pipeline_cache.h>
#include <KDGpu/pipeline_cache_options.h>

#include <KDGpu/kdgpu_export.h>

#include <span>
#include <vector>

namespace KDGpu {

class Adapter;

struct Device_t;

struct BufferOptions;
struct DeviceOptions;
struct GraphicsPipelineOptions;
struct SwapchainOptions;
struct TextureOptions;
struct BindGroupOptions;
struct BindGroupLayoutOptions;
struct BindGroupPoolOptions;
struct BindGroupEntry;
struct ComputePipelineOptions;
struct RayTracingPipelineOptions;
struct RenderPassOptions;
struct PipelineCacheOptions;

/*!
    \class Device
    \brief Represents a logical GPU device - the primary object for creating GPU resources
    \ingroup public
    \headerfile device.h <KDGpu/device.h>

    <b>Vulkan equivalent:</b> [VkDevice](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDevice.html)

    The Device is the central object in KDGpu applications. It represents a logical connection to
    a physical GPU (Adapter) and is the factory for creating all GPU resources: buffers, textures,
    pipelines, command recorders, and synchronization objects.

    <b>Key responsibilities:</b>
    - Create GPU resources (buffers, textures, images)
    - Create pipelines (graphics, compute, ray tracing)
    - Create shaders and pipeline layouts
    - Create command recorders for GPU work
    - Create synchronization primitives (fences, semaphores)
    - Create descriptor sets (bind groups)
    - Manage queues for submitting GPU commands
    .
    <br/>

    <b>Lifetime:</b> The Device should live for the entire time you need to render. All resources
    created from the device become invalid when the device is destroyed or moved. Typically you
    create one device at startup and keep it until shutdown.

    ## Usage

    <b>Creating a device:</b>

    \snippet kdgpu_doc_snippets.cpp device_creation

    <b>Accessing queues:</b>

    \snippet kdgpu_doc_snippets.cpp device_queue

    <b>Creating buffers:</b>

    \snippet kdgpu_doc_snippets.cpp device_create_buffer

    <b>Creating textures:</b>

    \snippet kdgpu_doc_snippets.cpp device_create_texture

    <b>Wait for GPU completion:</b>

    \snippet kdgpu_doc_snippets.cpp device_wait_idle

    <b>Multiple queues:</b>

    \snippet kdgpu_doc_snippets.cpp device_multi_queue

    ## Vulkan mapping:
    - Device::createBuffer() -> vkCreateBuffer() + vkAllocateMemory() + vkBindBufferMemory()
    - Device::createTexture() -> vkCreateImage() + vkAllocateMemory() + vkBindImageMemory()
    - Device::createGraphicsPipeline() -> vkCreateGraphicsPipelines()
    - Device::createComputePipeline() -> vkCreateComputePipelines()
    - Device::createShaderModule() -> vkCreateShaderModule()
    - Device::createCommandRecorder() -> vkAllocateCommandBuffers()
    - Device::createFence() -> vkCreateFence()
    - Device::createGpuSemaphore() -> vkCreateSemaphore()
    - Device::waitUntilIdle() -> vkDeviceWaitIdle()
    .
    <br/>

    \note KDGpu will try to request some device extensions by default KDGpu::getDefaultRequestedDeviceExtensions

    ## See also:
    \sa Adapter, DeviceOptions, Queue, Buffer, Texture, GraphicsPipeline, CommandRecorder
    \sa \ref kdgpu_api_overview
    \sa \ref kdgpu_vulkan_mapping
 */
class KDGPU_EXPORT Device
{
public:
    Device();
    ~Device();

    Device(Device &&) noexcept;
    Device &operator=(Device &&) noexcept;

    Device(const Device &) = delete;
    Device &operator=(const Device &) = delete;

    [[nodiscard]] Handle<Device_t> handle() const noexcept { return m_device; }
    [[nodiscard]] bool isValid() const noexcept { return m_device.isValid(); }

    operator Handle<Device_t>() const noexcept { return m_device; }

    [[nodiscard]] std::span<Queue> queues() { return m_queues; }

    void waitUntilIdle();

    [[nodiscard]] const Adapter *adapter() const;

    [[nodiscard]] Swapchain createSwapchain(const SwapchainOptions &options);
    [[nodiscard]] Texture createTexture(const TextureOptions &options);

    [[nodiscard]] Buffer createBuffer(const BufferOptions &options, const void *initialData = nullptr);

    [[nodiscard]] ShaderModule createShaderModule(const std::vector<uint32_t> &code);

    [[nodiscard]] RenderPass createRenderPass(const RenderPassOptions &options);

    [[nodiscard]] PipelineLayout createPipelineLayout(const PipelineLayoutOptions &options = PipelineLayoutOptions());

    [[nodiscard]] PipelineCache createPipelineCache(const PipelineCacheOptions &options = PipelineCacheOptions());

    [[nodiscard]] GraphicsPipeline createGraphicsPipeline(const GraphicsPipelineOptions &options);

    [[nodiscard]] ComputePipeline createComputePipeline(const ComputePipelineOptions &options);

    [[nodiscard]] RayTracingPipeline createRayTracingPipeline(const RayTracingPipelineOptions &options);

    [[nodiscard]] CommandRecorder createCommandRecorder(const CommandRecorderOptions &options = CommandRecorderOptions());

    [[nodiscard]] GpuSemaphore createGpuSemaphore(const GpuSemaphoreOptions &options = GpuSemaphoreOptions());

    [[nodiscard]] TimelineSemaphore createTimelineSemaphore(const TimelineSemaphoreOptions &options = TimelineSemaphoreOptions());

    [[nodiscard]] BindGroupLayout createBindGroupLayout(const BindGroupLayoutOptions &options);

    [[nodiscard]] BindGroupPool createBindGroupPool(const BindGroupPoolOptions &options);

    [[nodiscard]] BindGroup createBindGroup(const BindGroupOptions &options);

    [[nodiscard]] Sampler createSampler(const SamplerOptions &options = SamplerOptions());

    [[nodiscard]] Fence createFence(const FenceOptions &options = FenceOptions());

    [[nodiscard]] AccelerationStructure createAccelerationStructure(const AccelerationStructureOptions &options = AccelerationStructureOptions());

    [[nodiscard]] YCbCrConversion createYCbCrConversion(const YCbCrConversionOptions &options);

    [[nodiscard]] GraphicsApi *graphicsApi() const;

private:
    Device(Adapter *adapter, GraphicsApi *api, const DeviceOptions &options);

    GraphicsApi *m_api{ nullptr };
    Adapter *m_adapter{ nullptr };
    Handle<Device_t> m_device;
    std::vector<Queue> m_queues;

    friend class Adapter;
    friend class VulkanGraphicsApi;
};

} // namespace KDGpu
