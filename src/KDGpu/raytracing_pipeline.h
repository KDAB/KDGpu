/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>
#include <KDGpu/graphics_api.h>

namespace KDGpu {

struct Device_t;
struct RayTracingPipeline_t;
struct RayTracingPipelineOptions;

/**
 * @brief RayTracingPipeline
 * @ingroup public
 */
class KDGPU_EXPORT RayTracingPipeline
{
public:
    RayTracingPipeline();
    ~RayTracingPipeline();

    RayTracingPipeline(RayTracingPipeline &&);
    RayTracingPipeline &operator=(RayTracingPipeline &&);

    RayTracingPipeline(const RayTracingPipeline &) = delete;
    RayTracingPipeline &operator=(const RayTracingPipeline &) = delete;

    const Handle<RayTracingPipeline_t> &handle() const noexcept { return m_rayTracingPipeline; }
    bool isValid() const noexcept { return m_rayTracingPipeline.isValid(); }

    operator Handle<RayTracingPipeline_t>() const noexcept { return m_rayTracingPipeline; }

    std::vector<uint8_t> shaderGroupHandles(uint32_t firstGroup, uint32_t groupCount = 1) const;

private:
    explicit RayTracingPipeline(GraphicsApi *api,
                                const Handle<Device_t> &device,
                                const RayTracingPipelineOptions &options);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<RayTracingPipeline_t> m_rayTracingPipeline;

    friend class Device;
    friend KDGPU_EXPORT bool operator==(const RayTracingPipeline &, const RayTracingPipeline &);
};

KDGPU_EXPORT bool operator==(const RayTracingPipeline &a, const RayTracingPipeline &b);
KDGPU_EXPORT bool operator!=(const RayTracingPipeline &a, const RayTracingPipeline &b);

} // namespace KDGpu
