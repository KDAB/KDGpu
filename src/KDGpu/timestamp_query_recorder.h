/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/gpu_core.h>
#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>

#include <vector>

namespace KDGpu {

struct TimestampQueryRecorder_t;
struct Device_t;

class GraphicsApi;

/**
 * @brief TimestampQueryRecorder
 * @ingroup public
 */
class KDGPU_EXPORT TimestampQueryRecorder
{
public:
    ~TimestampQueryRecorder();

    TimestampQueryRecorder(TimestampQueryRecorder &&);
    TimestampQueryRecorder &operator=(TimestampQueryRecorder &&);

    TimestampQueryRecorder(const TimestampQueryRecorder &) = delete;
    TimestampQueryRecorder &operator=(const TimestampQueryRecorder &) = delete;

    const Handle<TimestampQueryRecorder_t> &handle() const noexcept { return m_timestampQueryRecorder; }
    bool isValid() const noexcept { return m_timestampQueryRecorder.isValid(); }

    operator Handle<TimestampQueryRecorder_t>() const noexcept { return m_timestampQueryRecorder; }

    TimestampIndex writeTimestamp(PipelineStageFlags flags);
    float timestampPeriod() const;

    void reset();
    std::vector<uint64_t> queryResults();

    uint64_t nsInterval(TimestampIndex begin, TimestampIndex end);

private:
    explicit TimestampQueryRecorder(GraphicsApi *api,
                                    const Handle<Device_t> &device,
                                    const Handle<TimestampQueryRecorder_t> &timestampQueryRecorder);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<TimestampQueryRecorder_t> m_timestampQueryRecorder;
    std::vector<uint64_t> m_lastResults;
    float m_timestampPeriod;

    friend class CommandRecorder;
};

} // namespace KDGpu
