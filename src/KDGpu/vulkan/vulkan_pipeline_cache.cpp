/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2026 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_pipeline_cache.h"
#include <KDGpu/vulkan/vulkan_resource_manager.h>
#include <KDGpu/vulkan/vulkan_formatters.h>

namespace KDGpu {

VulkanPipelineCache::VulkanPipelineCache(VkPipelineCache _pipelineCache,
                                         VulkanResourceManager *_vulkanResourceManager,
                                         const Handle<Device_t> &_deviceHandle)
    : pipelineCache(_pipelineCache)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
{
}

std::vector<uint8_t> VulkanPipelineCache::getData() const
{
    VulkanDevice *vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);

    size_t dataSize = 0;
    if (auto result = vkGetPipelineCacheData(vulkanDevice->device, pipelineCache, &dataSize, nullptr); result != VK_SUCCESS) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Error when getting pipeline cache data size: {}", result);
        return {};
    }

    std::vector<uint8_t> data(dataSize);
    if (VkResult result = vkGetPipelineCacheData(vulkanDevice->device, pipelineCache, &dataSize, data.data()); result != VK_SUCCESS) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Error when getting pipeline cache data: {}", result);
        return {};
    }

    return data;
}

bool VulkanPipelineCache::merge(const std::vector<RequiredHandle<PipelineCache_t>> &srcHandles) const
{
    if (srcHandles.empty())
        return false;

    VulkanDevice *vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);

    std::vector<VkPipelineCache> srcCaches;
    srcCaches.reserve(srcHandles.size());
    for (const auto &srcHandle : srcHandles) {
        if (!srcHandle.isValid()) {
            continue;
        }
        VulkanPipelineCache *srcCache = vulkanResourceManager->getPipelineCache(srcHandle);
        assert(srcCache != nullptr);
        srcCaches.push_back(srcCache->pipelineCache);
    }

    const VkResult result = vkMergePipelineCaches(vulkanDevice->device, pipelineCache,
                                                  static_cast<uint32_t>(srcCaches.size()), srcCaches.data());
    if (result != VK_SUCCESS) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Error when merging pipeline caches: {}", result);
    }

    return (result == VK_SUCCESS);
}

} // namespace KDGpu
