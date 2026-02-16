/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2026 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "pipeline_cache.h"

#include <KDGpu/api/graphics_api_impl.h>
#include <KDGpu/pipeline_cache_options.h>

namespace KDGpu {

PipelineCache::PipelineCache() = default;

PipelineCache::~PipelineCache()
{
    if (isValid())
        m_api->resourceManager()->deletePipelineCache(handle());
}

PipelineCache::PipelineCache(GraphicsApi *api, const Handle<Device_t> &device, const PipelineCacheOptions &options)
    : m_api(api)
    , m_device(device)
    , m_pipelineCache(m_api->resourceManager()->createPipelineCache(m_device, options))
{
}

PipelineCache::PipelineCache(PipelineCache &&other) noexcept
{
    m_api = std::exchange(other.m_api, nullptr);
    m_device = std::exchange(other.m_device, {});
    m_pipelineCache = std::exchange(other.m_pipelineCache, {});
}

PipelineCache &PipelineCache::operator=(PipelineCache &&other) noexcept
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deletePipelineCache(handle());

        m_api = std::exchange(other.m_api, nullptr);
        m_device = std::exchange(other.m_device, {});
        m_pipelineCache = std::exchange(other.m_pipelineCache, {});
    }
    return *this;
}

std::vector<uint8_t> PipelineCache::getData() const
{
    if (!isValid())
        return {};
    VulkanPipelineCache *vulkanPipelineCache = m_api->resourceManager()->getPipelineCache(handle());
    assert(vulkanPipelineCache != nullptr);
    return vulkanPipelineCache->getData();
}

bool PipelineCache::merge(const std::vector<Handle<PipelineCache_t>> &sources) const
{
    if (!isValid())
        return false;
    VulkanPipelineCache *vulkanPipelineCache = m_api->resourceManager()->getPipelineCache(handle());
    assert(vulkanPipelineCache != nullptr);
    return vulkanPipelineCache->merge(sources);
}

bool operator==(const PipelineCache &a, const PipelineCache &b)
{
    return a.m_api == b.m_api && a.m_device == b.m_device && a.m_pipelineCache == b.m_pipelineCache;
}

bool operator!=(const PipelineCache &a, const PipelineCache &b)
{
    return !(a == b);
}

} // namespace KDGpu
