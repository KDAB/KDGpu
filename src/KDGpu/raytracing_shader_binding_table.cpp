/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "raytracing_shader_binding_table.h"

#include <KDGpu/adapter.h>
#include <KDGpu/adapter_properties.h>
#include <KDGpu/buffer.h>
#include <KDGpu/buffer_options.h>
#include <KDGpu/device.h>

namespace KDGpu {

// https://docs.vulkan.org/spec/latest/chapters/raytracing.html#shader-binding-table
// https://www.willusher.io/graphics/2019/11/20/the-sbt-three-ways

RayTracingShaderBindingTable::RayTracingShaderBindingTable(Device *device, RayTracingShaderBindingTableOptions options)
    : m_options(options)
{
    const AdapterProperties &adapterProperties = device->adapter()->properties();
    m_shaderGroupAlignment = adapterProperties.rayTracingProperties.shaderGroupHandleAlignment;
    m_shaderGroupBaseAlignment = adapterProperties.rayTracingProperties.shaderGroupBaseAlignment;
    m_shaderGroupHandleSize = adapterProperties.rayTracingProperties.shaderGroupHandleSize;

    // As if this wasn't confusing enough already, we have to respect different alignments...
    m_handleSizeAligned = alignUp(m_shaderGroupHandleSize, m_shaderGroupAlignment);

    // Compute size of each region based on nbr of groups and alignment
    m_rayGenRegionSize = alignUp(m_handleSizeAligned, m_shaderGroupBaseAlignment);
    m_missRegionSize = alignUp(options.nbrMissShaders * m_handleSizeAligned, m_shaderGroupBaseAlignment);
    m_hitRegionSize = alignUp(options.nbrHitShaders * m_handleSizeAligned, m_shaderGroupBaseAlignment);

    m_buffer = device->createBuffer(BufferOptions{
            .size = m_rayGenRegionSize + m_missRegionSize + m_hitRegionSize,
            .usage = BufferUsageFlagBits::TransferSrcBit | BufferUsageFlagBits::TransferDstBit | BufferUsageFlagBits::ShaderBindingTableBit | BufferUsageFlagBits::ShaderDeviceAddressBit,
            .memoryUsage = MemoryUsage::CpuToGpu,
    });

    m_rayGenShaderRegion = StridedDeviceRegion{
        .buffer = m_buffer,
        .stride = m_rayGenRegionSize,
        .offset = 0,
        .size = m_rayGenRegionSize,
    };

    m_missShaderRegion = StridedDeviceRegion{
        .buffer = m_buffer,
        .stride = m_handleSizeAligned,
        .offset = m_rayGenRegionSize,
        .size = m_missRegionSize,
    };

    m_hitShaderRegion = StridedDeviceRegion{
        .buffer = m_buffer,
        .stride = m_handleSizeAligned,
        .offset = m_rayGenRegionSize + m_missRegionSize,
        .size = m_hitRegionSize,
    };
}

RayTracingShaderBindingTable::~RayTracingShaderBindingTable()
{
    m_buffer = {};
}

RayTracingShaderBindingTable::RayTracingShaderBindingTable(RayTracingShaderBindingTable &&other)
    : RayTracingShaderBindingTable()
{
    m_options = std::exchange(other.m_options, {});

    m_shaderGroupAlignment = std::exchange(other.m_shaderGroupAlignment, {});
    m_shaderGroupBaseAlignment = std::exchange(other.m_shaderGroupBaseAlignment, {});
    m_shaderGroupHandleSize = std::exchange(other.m_shaderGroupHandleSize, {});
    m_handleSizeAligned = std::exchange(other.m_handleSizeAligned, {});

    m_rayGenRegionSize = std::exchange(other.m_rayGenRegionSize, {});
    m_missRegionSize = std::exchange(other.m_missRegionSize, {});
    m_hitRegionSize = std::exchange(other.m_hitRegionSize, {});

    m_buffer = std::exchange(other.m_buffer, {});

    m_rayGenShaderRegion = std::exchange(other.m_rayGenShaderRegion, {});
    m_missShaderRegion = std::exchange(other.m_missShaderRegion, {});
    m_hitShaderRegion = std::exchange(other.m_hitShaderRegion, {});
}

RayTracingShaderBindingTable &RayTracingShaderBindingTable::operator=(RayTracingShaderBindingTable &&other)
{
    if (this != &other) {
        m_options = std::exchange(other.m_options, {});

        m_shaderGroupAlignment = std::exchange(other.m_shaderGroupAlignment, {});
        m_shaderGroupBaseAlignment = std::exchange(other.m_shaderGroupBaseAlignment, {});
        m_shaderGroupHandleSize = std::exchange(other.m_shaderGroupHandleSize, {});
        m_handleSizeAligned = std::exchange(other.m_handleSizeAligned, {});

        m_rayGenRegionSize = std::exchange(other.m_rayGenRegionSize, {});
        m_missRegionSize = std::exchange(other.m_missRegionSize, {});
        m_hitRegionSize = std::exchange(other.m_hitRegionSize, {});

        m_buffer = std::exchange(other.m_buffer, {});

        m_rayGenShaderRegion = std::exchange(other.m_rayGenShaderRegion, {});
        m_missShaderRegion = std::exchange(other.m_missShaderRegion, {});
        m_hitShaderRegion = std::exchange(other.m_hitShaderRegion, {});
    }
    return *this;
}

void RayTracingShaderBindingTable::addRayGenShaderGroup(const RayTracingPipeline &pipeline, uint32_t shaderGroupIndex)
{
    const std::vector<uint8_t> shaderGroupHandles = pipeline.shaderGroupHandles(shaderGroupIndex, 1);
    uint8_t *sbtDst = reinterpret_cast<uint8_t *>(m_buffer.map());
    std::memcpy(sbtDst, shaderGroupHandles.data(), shaderGroupHandles.size());
    m_buffer.unmap();
}

void RayTracingShaderBindingTable::addMissShaderGroup(const RayTracingPipeline &pipeline, uint32_t shaderGroupIndex, uint32_t entry)
{
    const std::vector<uint8_t> shaderGroupHandles = pipeline.shaderGroupHandles(shaderGroupIndex, 1);
    uint8_t *sbtDst = reinterpret_cast<uint8_t *>(m_buffer.map());
    std::memcpy(sbtDst + m_rayGenRegionSize + entry * m_handleSizeAligned, shaderGroupHandles.data(), shaderGroupHandles.size());
    m_buffer.unmap();
}

void RayTracingShaderBindingTable::addHitShaderGroup(const RayTracingPipeline &pipeline, uint32_t shaderGroupIndex, uint32_t entry)
{
    const std::vector<uint8_t> shaderGroupHandles = pipeline.shaderGroupHandles(shaderGroupIndex, 1);
    uint8_t *sbtDst = reinterpret_cast<uint8_t *>(m_buffer.map());
    std::memcpy(sbtDst + m_rayGenRegionSize + m_missRegionSize + entry * m_handleSizeAligned, shaderGroupHandles.data(), shaderGroupHandles.size());
    m_buffer.unmap();
}

// Return a value that satisfies x >= N * alignment
uint32_t RayTracingShaderBindingTable::alignUp(uint32_t x, size_t alignment) const
{
    return uint32_t((x + (uint32_t(alignment) - 1)) & ~uint32_t(alignment - 1));
}

} // namespace KDGpu
