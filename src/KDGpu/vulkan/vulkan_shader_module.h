/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/api/api_shader_module.h>

#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>

#include <vulkan/vulkan.h>

namespace KDGpu {

class VulkanResourceManager;

struct Device_t;

struct KDGPU_EXPORT VulkanShaderModule : public ApiShaderModule {
    explicit VulkanShaderModule(VkShaderModule _shaderModule,
                                VulkanResourceManager *_vulkanResourceManager,
                                const Handle<Device_t> _deviceHandle);

    VkShaderModule shaderModule{ VK_NULL_HANDLE };
    VulkanResourceManager *vulkanResourceManager{ nullptr };
    Handle<Device_t> deviceHandle;
};

} // namespace KDGpu
