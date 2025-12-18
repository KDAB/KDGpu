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

    // TODO: If initialData is set, upload this to the newly created buffer.
    // OR should this helper functionality go in a slightly higher layer that
    // knows about the concept of a frame so that it can correctly submit such commands
    // as part of the frame submission along with suitable memory barriers?
    [[nodiscard]] Buffer createBuffer(const BufferOptions &options, const void *initialData = nullptr);

    [[nodiscard]] ShaderModule createShaderModule(const std::vector<uint32_t> &code);

    [[nodiscard]] RenderPass createRenderPass(const RenderPassOptions &options);

    [[nodiscard]] PipelineLayout createPipelineLayout(const PipelineLayoutOptions &options = PipelineLayoutOptions());

    [[nodiscard]] GraphicsPipeline createGraphicsPipeline(const GraphicsPipelineOptions &options);

    [[nodiscard]] ComputePipeline createComputePipeline(const ComputePipelineOptions &options);

    [[nodiscard]] RayTracingPipeline createRayTracingPipeline(const RayTracingPipelineOptions &options);

    [[nodiscard]] CommandRecorder createCommandRecorder(const CommandRecorderOptions &options = CommandRecorderOptions());

    [[nodiscard]] GpuSemaphore createGpuSemaphore(const GpuSemaphoreOptions &options = GpuSemaphoreOptions());

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
