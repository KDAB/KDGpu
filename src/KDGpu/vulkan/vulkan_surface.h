/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/api/api_surface.h>
#include <KDGpu/kdgpu_export.h>

#include <vulkan/vulkan.h>

namespace KDGpu {

/**
 * @brief VulkanSurface
 * \ingroup vulkan
 *
 */
struct KDGPU_EXPORT VulkanSurface : public ApiSurface {
    explicit VulkanSurface(VkSurfaceKHR _surface, VkInstance _instance, bool _isOwned = true);

    VkSurfaceKHR surface{ VK_NULL_HANDLE };
    VkInstance instance{ VK_NULL_HANDLE };
    bool isOwned{ true };
};

} // namespace KDGpu
