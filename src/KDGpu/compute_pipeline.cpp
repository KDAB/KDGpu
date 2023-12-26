/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "compute_pipeline.h"
#include <KDGpu/graphics_api.h>
#include <KDGpu/compute_pipeline_options.h>

namespace KDGpu {

ComputePipeline::ComputePipeline() = default;

ComputePipeline::~ComputePipeline()
{
    if (isValid())
        m_api->resourceManager()->deleteComputePipeline(handle());
}

ComputePipeline::ComputePipeline(GraphicsApi *api,
                                 const Handle<Device_t> &device,
                                 const ComputePipelineOptions &options)
    : m_api(api)
    , m_device(device)
    , m_computePipeline(m_api->resourceManager()->createComputePipeline(m_device, options))
{
}

ComputePipeline::ComputePipeline(ComputePipeline &&other)
{
    m_api = std::exchange(other.m_api, nullptr);
    m_device = std::exchange(other.m_device, {});
    m_computePipeline = std::exchange(other.m_computePipeline, {});
}

ComputePipeline &ComputePipeline::operator=(ComputePipeline &&other)
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteComputePipeline(handle());

        m_api = std::exchange(other.m_api, nullptr);
        m_device = std::exchange(other.m_device, {});
        m_computePipeline = std::exchange(other.m_computePipeline, {});
    }
    return *this;
}

bool operator==(const ComputePipeline &a, const ComputePipeline &b)
{
    return a.m_api == b.m_api && a.m_device == b.m_device && a.m_computePipeline == b.m_computePipeline;
}

bool operator!=(const ComputePipeline &a, const ComputePipeline &b)
{
    return !(a == b);
}

} // namespace KDGpu
