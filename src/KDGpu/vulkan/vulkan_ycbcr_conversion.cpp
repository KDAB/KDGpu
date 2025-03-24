/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_ycbcr_conversion.h"

namespace KDGpu {

#if defined(VK_KHR_sampler_ycbcr_conversion)
VulkanYCbCrConversion::VulkanYCbCrConversion(VkSamplerYcbcrConversionKHR _yCbCrConversion,
                                             const Handle<Device_t> &_deviceHandle)
    : yCbCrConversion(_yCbCrConversion)
    , deviceHandle(_deviceHandle)
{
}
#endif

} // namespace KDGpu
