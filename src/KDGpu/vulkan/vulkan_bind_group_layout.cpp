/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_bind_group_layout.h"

#include <algorithm>
namespace KDGpu {

VulkanBindGroupLayout::VulkanBindGroupLayout(VkDescriptorSetLayout _descriptorSetLayout,
                                             const Handle<Device_t> &_deviceHandle,
                                             const std::vector<ResourceBindingLayout> &_bindings)
    : descriptorSetLayout(_descriptorSetLayout)
    , deviceHandle(_deviceHandle)
    , bindings(_bindings)
{
    // Sort Bindings by their index to ease comparison
    std::sort(bindings.begin(), bindings.end(), [](const auto &b1, const auto &b2) {
        return b1.binding < b2.binding;
    });
}

bool VulkanBindGroupLayout::isCompatibleWith(const ApiBindGroupLayout &other) const
{
    const VulkanBindGroupLayout *otherVkLayout = static_cast<const VulkanBindGroupLayout *>(&other);
    return bindings == otherVkLayout->bindings;
}

} // namespace KDGpu
