/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/api/api_sampler.h>
#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>
#include <vulkan/vulkan.h>

namespace KDGpu {

class VulkanResourceManager;
struct Device_t;

struct KDGPU_EXPORT VulkanSampler : public ApiSampler {

    explicit VulkanSampler(VkSampler _sampler, const Handle<Device_t> &_deviceHandle);

    VkSampler sampler{ VK_NULL_HANDLE };
    Handle<Device_t> deviceHandle;
};

} // namespace KDGpu
