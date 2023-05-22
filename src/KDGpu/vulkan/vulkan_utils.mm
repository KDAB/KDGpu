/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#import <QuartzCore/CAMetalLayer.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_metal.h>

#include <KDGpu/surface_options.h>

#include <KDGpu/utils/logging.h>

VkSurfaceKHR createVulkanSurface(VkInstance instance, const KDGpu::SurfaceOptions &options)
{
    VkSurfaceKHR vkSurface{ VK_NULL_HANDLE };

    VkMetalSurfaceCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
    createInfo.pLayer = options.layer;
    if (vkCreateMetalSurfaceEXT(instance, &createInfo, nullptr, &vkSurface) != VK_SUCCESS) {
        SPDLOG_LOGGER_WARN(KDGpu::Logger::logger(), "Failed to create Vulkan surface for Metal layer");
        return {};
    }

    return vkSurface;
}
