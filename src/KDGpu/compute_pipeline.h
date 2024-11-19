/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>
#include <KDGpu/graphics_api.h>

namespace KDGpu {

struct Device_t;
struct ComputePipeline_t;
struct ComputePipelineOptions;

/**
 * @brief ComputePipeline
 * @ingroup public
 */
class KDGPU_EXPORT ComputePipeline
{
public:
    ComputePipeline();
    ~ComputePipeline();

    ComputePipeline(ComputePipeline &&);
    ComputePipeline &operator=(ComputePipeline &&);

    ComputePipeline(const ComputePipeline &) = delete;
    ComputePipeline &operator=(const ComputePipeline &) = delete;

    const Handle<ComputePipeline_t> &handle() const noexcept { return m_computePipeline; }
    bool isValid() const noexcept { return m_computePipeline.isValid(); }

    operator Handle<ComputePipeline_t>() const noexcept { return m_computePipeline; }

private:
    explicit ComputePipeline(GraphicsApi *api,
                             const Handle<Device_t> &device,
                             const ComputePipelineOptions &options);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<ComputePipeline_t> m_computePipeline;

    friend class Device;
    friend KDGPU_EXPORT bool operator==(const ComputePipeline &, const ComputePipeline &);
};

KDGPU_EXPORT bool operator==(const ComputePipeline &a, const ComputePipeline &b);
KDGPU_EXPORT bool operator!=(const ComputePipeline &a, const ComputePipeline &b);

} // namespace KDGpu
