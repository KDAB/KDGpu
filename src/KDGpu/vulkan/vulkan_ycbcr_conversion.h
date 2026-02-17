/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>

#include <vulkan/vulkan.h>

namespace KDGpu {

struct Device_t;

struct KDGPU_EXPORT VulkanYCbCrConversion {

#if VK_KHR_sampler_ycbcr_conversion
    explicit VulkanYCbCrConversion(VkSamplerYcbcrConversionKHR _yCbCrConversion,
                                   const Handle<Device_t> &_deviceHandle);

    VkSamplerYcbcrConversionKHR yCbCrConversion{ VK_NULL_HANDLE };
    Handle<Device_t> deviceHandle;
#endif
};

} // namespace KDGpu
