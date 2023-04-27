/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_bind_group_layout.h"

namespace KDGpu {

VulkanBindGroupLayout::VulkanBindGroupLayout(VkDescriptorSetLayout _descriptorSetLayout,
                                             const Handle<Device_t> &_deviceHandle)
    : descriptorSetLayout(_descriptorSetLayout)
    , deviceHandle(_deviceHandle)
{
}

} // namespace KDGpu
