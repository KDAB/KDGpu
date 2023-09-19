/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "timestamp_query_recorder.h"
#include <KDGpu/graphics_api.h>
#include <KDGpu/api/api_timestamp_query_recorder.h>

namespace KDGpu {

TimestampQueryRecorder::TimestampQueryRecorder(GraphicsApi *api,
                                               const Handle<Device_t> &device,
                                               const Handle<TimestampQueryRecorder_t> &timestampQueryRecorder)
    : m_api(api)
    , m_device(device)
    , m_timestampQueryRecorder(timestampQueryRecorder)
{
    m_timestampPeriod = m_api->resourceManager()->getTimestampQueryRecorder(timestampQueryRecorder)->timestampPeriod();
}

TimestampQueryRecorder::~TimestampQueryRecorder()
{
    if (isValid())
        m_api->resourceManager()->deleteTimestampQueryRecorder(handle());
}

TimestampQueryRecorder::TimestampQueryRecorder(TimestampQueryRecorder &&other)
{
    m_api = other.m_api;
    m_device = other.m_device;
    m_timestampQueryRecorder = other.m_timestampQueryRecorder;
    m_timestampPeriod = other.m_timestampPeriod;
}

TimestampQueryRecorder &TimestampQueryRecorder::operator=(TimestampQueryRecorder &&other)
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteTimestampQueryRecorder(handle());

        m_api = other.m_api;
        m_device = other.m_device;
        m_timestampQueryRecorder = other.m_timestampQueryRecorder;
        m_timestampPeriod = other.m_timestampPeriod;
    }
    return *this;
}

TimestampIndex TimestampQueryRecorder::writeTimestamp(PipelineStageFlags flags)
{
    auto apiTimestampRecorder = m_api->resourceManager()->getTimestampQueryRecorder(m_timestampQueryRecorder);
    return apiTimestampRecorder->writeTimestamp(flags);
}

float TimestampQueryRecorder::timestampPeriod() const
{
    return m_timestampPeriod;
}

void TimestampQueryRecorder::reset()
{
    auto apiTimestampRecorder = m_api->resourceManager()->getTimestampQueryRecorder(m_timestampQueryRecorder);
    apiTimestampRecorder->reset();
    m_lastResults = {};
}

std::vector<uint64_t> TimestampQueryRecorder::queryResults()
{
    auto apiTimestampRecorder = m_api->resourceManager()->getTimestampQueryRecorder(m_timestampQueryRecorder);
    m_lastResults = apiTimestampRecorder->queryResults();
    return m_lastResults;
}

uint64_t TimestampQueryRecorder::nsInterval(TimestampIndex begin, TimestampIndex end)
{
    if (m_lastResults.empty())
        queryResults();

    if (begin < m_lastResults.size() && end < m_lastResults.size())
        return (m_lastResults[end] - m_lastResults[begin]) * uint64_t(m_timestampPeriod);

    return 0;
}

} // namespace KDGpu
