/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2026 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>
#include <KDGpu/graphics_api.h>

#include <vector>
#include <cstdint>

namespace KDGpu {

struct Device_t;
struct PipelineCache_t;
struct PipelineCacheOptions;

/**
 * @brief PipelineCache
 * @ingroup public
 *
 * A PipelineCache allows the result of pipeline construction to be reused between pipelines
 * and between runs of an application. Reusing a pipeline cache can significantly reduce
 * pipeline construction time, improving application startup and runtime performance.
 *
 * Pipeline cache data can be retrieved using getData() and saved to disk, then loaded
 * in subsequent runs using PipelineCacheOptions::initialData.
 *
 * Multiple pipeline caches can be merged using merge() to consolidate cached data.
 *
 * @warning The pipeline cache is externally synchronized. When creating pipelines on multiple
 * threads with the same cache, or when calling merge() while pipelines are being created,
 * external synchronization (e.g., a mutex) must be used to ensure thread safety.
 *
 * @sa PipelineCacheOptions
 * @sa Device::createPipelineCache
 */
class KDGPU_EXPORT PipelineCache
{
public:
    PipelineCache();
    ~PipelineCache();

    PipelineCache(PipelineCache &&) noexcept;
    PipelineCache &operator=(PipelineCache &&) noexcept;

    PipelineCache(const PipelineCache &) = delete;
    PipelineCache &operator=(const PipelineCache &) = delete;

    const Handle<PipelineCache_t> &handle() const noexcept { return m_pipelineCache; }
    bool isValid() const noexcept { return m_pipelineCache.isValid(); }

    operator Handle<PipelineCache_t>() const noexcept { return m_pipelineCache; }

    /**
     * @brief Retrieves the data from the pipeline cache
     * @return A vector containing the cached pipeline data, which can be saved and reloaded later
     *
     * The returned data is opaque and implementation-specific. It should be treated as a
     * binary blob that can be persisted to disk and loaded in subsequent runs.
     */
    std::vector<uint8_t> getData() const;

    /**
     * @brief Merges data from source pipeline caches into this cache
     * @param sources Vector of source pipeline cache handles to merge from
     *
     * After merging, this cache will contain the union of all pipeline data from this cache
     * and all source caches. The source caches remain valid and unchanged.
     *
     * @warning This operation requires external synchronization if any of the caches (this
     * cache or any source caches) are being used concurrently for pipeline creation.
     */
    bool merge(const std::vector<Handle<PipelineCache_t>> &sources) const;

private:
    explicit PipelineCache(GraphicsApi *api, const Handle<Device_t> &device, const PipelineCacheOptions &options);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<PipelineCache_t> m_pipelineCache;

    friend class Device;
    friend KDGPU_EXPORT bool operator==(const PipelineCache &, const PipelineCache &);
};

KDGPU_EXPORT bool operator==(const PipelineCache &a, const PipelineCache &b);
KDGPU_EXPORT bool operator!=(const PipelineCache &a, const PipelineCache &b);

} // namespace KDGpu
