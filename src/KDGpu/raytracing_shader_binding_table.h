/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/raytracing_pass_command_recorder.h>
#include <KDGpu/buffer.h>

namespace KDGpu {

class Device;
class RayTracingPipeline;

struct RayTracingShaderBindingTableOptions {
    size_t nbrMissShaders{ 0 };
    size_t nbrHitShaders{ 0 };
};

/**
 * @brief RayTracingShaderBindingTable
 * @ingroup public
 */
class KDGPU_EXPORT RayTracingShaderBindingTable
{
public:
    RayTracingShaderBindingTable() = default;
    explicit RayTracingShaderBindingTable(Device *device, RayTracingShaderBindingTableOptions options);
    ~RayTracingShaderBindingTable();

    RayTracingShaderBindingTable(RayTracingShaderBindingTable &&) noexcept;
    RayTracingShaderBindingTable &operator=(RayTracingShaderBindingTable &&) noexcept;

    RayTracingShaderBindingTable(const RayTracingShaderBindingTable &) = delete;
    RayTracingShaderBindingTable &operator=(const RayTracingShaderBindingTable &) = delete;

    StridedDeviceRegion rayGenShaderRegion() const { return m_rayGenShaderRegion; }
    StridedDeviceRegion missShaderRegion() const { return m_missShaderRegion; }
    StridedDeviceRegion hitShaderRegion() const { return m_hitShaderRegion; }

    void addRayGenShaderGroup(const RayTracingPipeline &pipeline, uint32_t shaderGroupIndex);
    void addHitShaderGroup(const RayTracingPipeline &pipeline, uint32_t shaderGroupIndex, uint32_t entry = 0);
    void addMissShaderGroup(const RayTracingPipeline &pipeline, uint32_t shaderGroupIndex, uint32_t entry = 0);

    Buffer &buffer() { return m_buffer; }

private:
    uint32_t alignUp(uint32_t x, size_t alignment) const;

    RayTracingShaderBindingTableOptions m_options;
    uint32_t m_shaderGroupAlignment;
    uint32_t m_shaderGroupBaseAlignment;
    uint32_t m_shaderGroupHandleSize;
    uint32_t m_handleSizeAligned;
    size_t m_rayGenRegionSize;
    size_t m_missRegionSize;
    size_t m_hitRegionSize;
    Buffer m_buffer;

    StridedDeviceRegion m_rayGenShaderRegion;
    StridedDeviceRegion m_missShaderRegion;
    StridedDeviceRegion m_hitShaderRegion;
};

} // namespace KDGpu
