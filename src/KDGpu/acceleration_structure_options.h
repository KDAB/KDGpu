/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <span>

#include <KDGpu/gpu_core.h>
#include <KDGpu/handle.h>

namespace KDGpu {

struct AccelerationStructure_t;
struct Buffer_t;

struct AccelerationStructureGeometryTrianglesData {
    Format vertexFormat{ Format::UNDEFINED };
    Handle<Buffer_t> vertexData;
    size_t vertexStride{ 0 };
    uint32_t maxVertex{ 0 };
    IndexType indexType{ IndexType::Uint16 };
    Handle<Buffer_t> indexData;
    Handle<Buffer_t> transformData;
};

struct AccelerationStructureGeometryAabbsData {
    Handle<Buffer_t> data;
    size_t stride{ 0 };
};

struct AccelerationStructureGeometryInstancesData {
    Handle<Buffer_t> data;
};

using AccelerationStructureGeometry = std::variant<AccelerationStructureGeometryTrianglesData, AccelerationStructureGeometryAabbsData, AccelerationStructureGeometryInstancesData>;

struct AccelerationStructureOptions {
    BuildAccelerationStructureMode mode{ BuildAccelerationStructureMode::Build };
    AccelerationStructureType type{ AccelerationStructureType::TopLevel };
    std::vector<AccelerationStructureGeometry> geometries;
    Handle<AccelerationStructure_t> sourceStructure;
    Handle<AccelerationStructure_t> destinationStructure;
};

struct AccelerationStructureBuildRangeInfo {
    uint32_t primitiveCount{ 0 };
    uint32_t primitiveOffset{ 0 };
    uint32_t firstVertex{ 0 };
    uint32_t transformOffset{ 0 };
};

struct BuildAccelerationStructureOptions {
    std::vector<AccelerationStructureOptions> buildGeometryInfos;
    std::vector<AccelerationStructureBuildRangeInfo> buildRangeInfos;
};

} // namespace KDGpu
