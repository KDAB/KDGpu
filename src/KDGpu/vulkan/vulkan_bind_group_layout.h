/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/api/api_bind_group_layout.h>
#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>
#include <vulkan/vulkan.h>

namespace KDGpu {

class VulkanResourceManager;
struct Device_t;

struct KDGPU_EXPORT VulkanBindGroupLayout : public ApiBindGroupLayout {
    explicit VulkanBindGroupLayout(VkDescriptorSetLayout _descriptorSetLayout,
                                   const Handle<Device_t> &_deviceHandle);

    VkDescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };
    Handle<Device_t> deviceHandle;
};

} // namespace KDGpu
