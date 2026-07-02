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
#include <KDGpu/graphics_api.h>

#include <vector>

namespace KDGpu {

struct TimestampQueryRecorder_t;
struct Device_t;

/**
 * @brief TimestampQueryRecorder
 * @ingroup public
 */
class KDGPU_EXPORT TimestampQueryRecorder
{
public:
    TimestampQueryRecorder();
    ~TimestampQueryRecorder();

    TimestampQueryRecorder(TimestampQueryRecorder &&) noexcept;
    TimestampQueryRecorder &operator=(TimestampQueryRecorder &&) noexcept;

    TimestampQueryRecorder(const TimestampQueryRecorder &) = delete;
    TimestampQueryRecorder &operator=(const TimestampQueryRecorder &) = delete;

    [[nodiscard]] const Handle<TimestampQueryRecorder_t> &handle() const noexcept { return m_timestampQueryRecorder; }
    [[nodiscard]] bool isValid() const noexcept { return m_timestampQueryRecorder.isValid(); }

    operator Handle<TimestampQueryRecorder_t>() const noexcept { return m_timestampQueryRecorder; }

    TimestampIndex writeTimestamp(PipelineStageFlags flags);
    [[nodiscard]] float timestampPeriod() const;

    void reset();
    [[nodiscard]] std::vector<uint64_t> queryResults();

    [[nodiscard]] uint64_t nsInterval(TimestampIndex begin, TimestampIndex end);

private:
    explicit TimestampQueryRecorder(GraphicsApi *api,
                                    const Handle<Device_t> &device,
                                    const Handle<TimestampQueryRecorder_t> &timestampQueryRecorder);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<TimestampQueryRecorder_t> m_timestampQueryRecorder;
    std::vector<uint64_t> m_lastResults;
    float m_timestampPeriod{ 1.0f };

    friend class CommandRecorder;
};

} // namespace KDGpu
