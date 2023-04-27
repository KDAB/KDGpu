/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_sampler.h"

namespace KDGpu {

VulkanSampler::VulkanSampler(VkSampler _sampler, const Handle<Device_t> &_deviceHandle)
    : sampler(_sampler)
    , deviceHandle(_deviceHandle)
{
}

} // namespace KDGpu
