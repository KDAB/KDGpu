/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/gpu_core.h>
#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>
#include <KDGpu/graphics_api.h>

#include <span>

namespace KDGpu {

struct BindGroup_t;
struct Buffer_t;
struct Device_t;
struct PushConstantRange;
struct PipelineLayout_t;
struct RayTracingPipeline_t;
struct RayTracingPassCommandRecorder_t;
struct BindGroupEntry;

struct StridedDeviceRegion {
    Handle<Buffer_t> buffer;
    DeviceSize stride{ 0 };
    DeviceSize offset{ 0 };
    DeviceSize size{ 0 };
};

struct RayTracingCommand {
    StridedDeviceRegion raygenShaderBindingTable;
    StridedDeviceRegion missShaderBindingTable;
    StridedDeviceRegion hitShaderBindingTable;
    StridedDeviceRegion callableShaderBindingTable;
    Extent3D extent;
};

struct RayTracingPassCommandRecorderOptions {
};

/**
 * @brief RayTracingPassCommandRecorder
 * @ingroup public
 */
class KDGPU_EXPORT RayTracingPassCommandRecorder
{
public:
    ~RayTracingPassCommandRecorder();

    RayTracingPassCommandRecorder(RayTracingPassCommandRecorder &&) noexcept;
    RayTracingPassCommandRecorder &operator=(RayTracingPassCommandRecorder &&) noexcept;

    RayTracingPassCommandRecorder(const RayTracingPassCommandRecorder &) = delete;
    RayTracingPassCommandRecorder &operator=(const RayTracingPassCommandRecorder &) = delete;

    const Handle<RayTracingPassCommandRecorder_t> &handle() const noexcept { return m_rayTracingCommandRecorder; }
    bool isValid() const noexcept { return m_rayTracingCommandRecorder.isValid(); }

    operator Handle<RayTracingPassCommandRecorder_t>() const noexcept { return m_rayTracingCommandRecorder; }

    void setPipeline(const Handle<RayTracingPipeline_t> &pipeline);

    void setBindGroup(uint32_t group,
                      const Handle<BindGroup_t> &bindGroup,
                      const Handle<PipelineLayout_t> &pipelineLayout = Handle<PipelineLayout_t>(),
                      std::span<const uint32_t> dynamicBufferOffsets = {});

    void traceRays(const RayTracingCommand &rayTracingCommand);

    void pushConstant(const PushConstantRange &constantRange, const void *data);

    void pushBindGroup(uint32_t group,
                       std::span<const BindGroupEntry> bindGroupEntries,
                       const Handle<PipelineLayout_t> &pipelineLayout = Handle<PipelineLayout_t>());

    void end();

private:
    explicit RayTracingPassCommandRecorder(GraphicsApi *api,
                                           const Handle<Device_t> &device,
                                           const Handle<RayTracingPassCommandRecorder_t> &rayTracingPassRecorder);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<RayTracingPassCommandRecorder_t> m_rayTracingCommandRecorder;

    friend class CommandRecorder;
};

} // namespace KDGpu
