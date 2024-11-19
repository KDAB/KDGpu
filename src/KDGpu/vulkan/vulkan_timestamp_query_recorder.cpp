/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_timestamp_query_recorder.h"

#include <KDGpu/vulkan/vulkan_resource_manager.h>
#include <KDGpu/vulkan/vulkan_enums.h>
#include <KDGpu/vulkan/vulkan_device.h>
#include <KDGpu/vulkan/vulkan_formatters.h>

namespace KDGpu {

VulkanTimestampQueryRecorder::VulkanTimestampQueryRecorder(VkCommandBuffer _commandBuffer,
                                                           VulkanResourceManager *_vulkanResourceManager,
                                                           const Handle<Device_t> &_deviceHandle,
                                                           uint32_t _startQuery,
                                                           uint32_t _maxQueryCount)
    : commandBuffer(_commandBuffer)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
    , startQuery(_startQuery)
    , maxQueryCount(_maxQueryCount)
{
    VulkanDevice *vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);
    VulkanAdapter *adapter = vulkanResourceManager->getAdapter(vulkanDevice->adapterHandle);

    m_timestampPeriod = adapter->queryAdapterProperties().limits.timestampPeriod;

    reset();
}

TimestampIndex VulkanTimestampQueryRecorder::writeTimestamp(PipelineStageFlags flags)
{
    if (queryCount == maxQueryCount) {
        SPDLOG_LOGGER_WARN(Logger::logger(), "TimestampQueryRecorder query count exceeded, overwriting last query");
    }

    VulkanDevice *vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);
    const TimestampIndex queryIndex = startQuery + std::min(queryCount, maxQueryCount - 1);
    vkCmdWriteTimestamp(commandBuffer,
                        pipelineStageFlagsToVkPipelineStageFlagBits(flags),
                        vulkanDevice->timestampQueryPool,
                        queryIndex);
    queryCount = std::min(queryCount + 1, maxQueryCount);
    return queryIndex;
}

std::vector<uint64_t> VulkanTimestampQueryRecorder::queryResults()
{
    VulkanDevice *vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);

    struct QueryResult {
        uint64_t result;
        uint64_t available;
    };

    std::vector<QueryResult> results;
    results.resize(queryCount);

    VkResult result = vkGetQueryPoolResults(vulkanDevice->device,
                                            vulkanDevice->timestampQueryPool,
                                            startQuery,
                                            queryCount,
                                            results.size() * sizeof(QueryResult),
                                            results.data(),
                                            sizeof(QueryResult),
                                            VK_QUERY_RESULT_WITH_AVAILABILITY_BIT | VK_QUERY_RESULT_64_BIT);

    if (result == VK_NOT_READY) {
        SPDLOG_LOGGER_WARN(Logger::logger(), "Timestamp query results not ready");
    } else if (result != VK_SUCCESS) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Error when retrieving timestamp query results: {}", result);
        return {};
    }

    std::vector<uint64_t> finalResults;
    finalResults.reserve(results.size());

    VulkanAdapter *adapter = vulkanResourceManager->getAdapter(vulkanDevice->adapterHandle);

    for (const QueryResult &r : results) {
        const uint64_t v = r.available == 0 ? 0 : r.result;
        finalResults.emplace_back(v);
    }

    return finalResults;
}

void VulkanTimestampQueryRecorder::reset()
{
    VulkanDevice *vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);

    vkCmdResetQueryPool(commandBuffer, vulkanDevice->timestampQueryPool, startQuery, maxQueryCount);
    queryCount = 0;

    // Requires hostQueryReset feature enabled on the device
    // vkResetQueryPool(vulkanDevice->device, vulkanDevice->timestampQueryPool, startQuery, maxQueryCount);
}

float VulkanTimestampQueryRecorder::timestampPeriod() const
{
    return m_timestampPeriod;
}

} // namespace KDGpu
